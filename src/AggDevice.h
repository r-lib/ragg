#include "ragg.h"
#include "text_renderer.h"

#include "agg_math_stroke.h"

#include "agg_ellipse.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
//
#include "agg_span_interpolator_linear.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_allocator.h"
//
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"

#ifndef AGGDEV_INCLUDED
#define AGGDEV_INCLUDED

/* Base class for graphic device interface to AGG. See AggDevice.cpp for 
 * implementation details. 
 * 
 * Specific devices should subclass this and provide their own buffer and 
 * savePage() methods (at least), while the drawing methods should work 
 * regardless. The base class outputs images in ppm format which is not realy
 * a usable format. See png and tiff versions for actual usable classes.
 */
template<class PIXFMT, class R_COLOR = agg::rgba8>
class AggDevice {
public:
  typedef PIXFMT pixfmt_type;
  typedef agg::renderer_base<pixfmt_type> renbase_type;
  typedef agg::renderer_scanline_aa_solid<renbase_type> renderer_solid;
  typedef agg::renderer_scanline_bin_solid<renbase_type> renderer_bin;
  
  static const int bytes_per_pixel = pixfmt_type::pix_width;
  
  bool can_capture = false;
  
  int width;
  int height;
  double clip_left;
  double clip_right;
  double clip_top;
  double clip_bottom;
  
  renbase_type renderer;
  pixfmt_type* pixf;
  agg::rendering_buffer rbuf;
  unsigned char* buffer;
  
  int pageno;
  std::string file;
  R_COLOR background;
  int background_int;
  double pointsize;
  double res_mod;
  double lwd_mod;
  
  TextRenderer t_ren;
  
  // Lifecycle methods
  AggDevice(const char* fp, int w, int h, double ps, int bg, double res);
  virtual ~AggDevice();
  void newPage();
  void close();
  virtual bool savePage();
  SEXP capture();
  
  // Behaviour
  void clipRect(double x0, double y0, double x1, double y1);
  double stringWidth(const char *str, const char *family, int face, 
                     double size);
  void charMetric(int c, const char *family, int face, double size,
                  double *ascent, double *descent, double *width);
  
  // Drawing Methods
  void drawCircle(double x, double y, double r, int fill, int col, double lwd, 
                  int lty, R_GE_lineend lend);
  void drawRect(double x0, double y0, double x1, double y1, int fill, int col, 
                double lwd, int lty, R_GE_lineend lend);
  void drawPolygon(int n, double *x, double *y, int fill, int col, double lwd, 
                   int lty, R_GE_lineend lend, R_GE_linejoin ljoin, 
                   double lmitre);
  void drawLine(double x1, double y1, double x2, double y2, int col, double lwd, 
                int lty, R_GE_lineend lend);
  void drawPolyline(int n, double* x, double* y, int col, double lwd, int lty,
                    R_GE_lineend lend, R_GE_linejoin ljoin, double lmitre);
  void drawPath(int npoly, int* nper, double* x, double* y, int col, int fill, 
                double lwd, int lty, R_GE_lineend lend, R_GE_linejoin ljoin, 
                double lmitre, bool evenodd);
  void drawRaster(unsigned int *raster, int w, int h, double x, double y, 
                  double final_width, double final_height, double rot, 
                  bool interpolate);
  void drawText(double x, double y, const char *str, const char *family, 
                int face, double size, double rot, double hadj, int col);
  
private:
  virtual inline R_COLOR convertColour(unsigned int col) {
    return R_COLOR(R_RED(col), R_GREEN(col), R_BLUE(col), R_ALPHA(col)).premultiply();
  }
  inline bool visibleColour(unsigned int col) {
    return col != NA_INTEGER && R_ALPHA(col) != 0;
  }
  inline agg::line_cap_e convertLineend(R_GE_lineend lend) {
    switch (lend) {
    case GE_ROUND_CAP:
      return agg::round_cap;
    case GE_BUTT_CAP:
      return agg::butt_cap;
    case GE_SQUARE_CAP:
      return agg::square_cap;
    }
    
    //should never happen
    return agg::square_cap;
  }
  inline agg::line_join_e convertLinejoin(R_GE_linejoin ljoin) {
    switch(ljoin) {
    case GE_ROUND_JOIN:
      return agg::round_join;
    case GE_MITRE_JOIN:
      return agg::miter_join;
    case GE_BEVEL_JOIN:
      return agg::bevel_join;
    }
    
    //should never happen
    return agg::round_join;
  }
  template<class T>
  void makeDash(T &dash_conv, int lty, double lwd) {
    dash_conv.remove_all_dashes();
    double dash, gap;
    for(int i = 0 ; i < 8 && lty & 15 ; i += 2) {
      dash = (lty & 15) * lwd;
      lty = lty>>4;
      gap = (lty & 15) * lwd;
      lty = lty>>4;
      dash_conv.add_dash(dash, gap);
    }
  }
};

// IMPLIMENTATION --------------------------------------------------------------

// LIFECYCLE -------------------------------------------------------------------

/* The initialiser takes care of setting up the buffer, and caching a pixel 
 * formatter and renderer.
 */
template<class PIXFMT, class R_COLOR>
AggDevice<PIXFMT, R_COLOR>::AggDevice(const char* fp, int w, int h, double ps, 
                                      int bg, double res) : 
  width(w),
  height(h),
  pageno(0),
  file(fp),
  background_int(bg),
  pointsize(ps),
  res_mod(res / 72.0),
  lwd_mod(res / 96.0),
  t_ren()
{
  buffer = new unsigned char[width * height * bytes_per_pixel];
  rbuf = agg::rendering_buffer(buffer, width, height, width * bytes_per_pixel);
  pixf = new pixfmt_type(rbuf);
  renderer = renbase_type(*pixf);
  background = convertColour(background_int);
  renderer.clear(background);
}
template<class PIXFMT, class R_COLOR>
AggDevice<PIXFMT, R_COLOR>::~AggDevice() {
  delete pixf;
  delete [] buffer;
}

/* newPage() should not need to be overwritten as long the class have an 
 * appropriate savePage() method. For scrren devices it may make sense to change
 * it for performance
 */
template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::newPage() {
  if (pageno != 0) {
    if (!savePage()) {
      Rf_warning("agg could not write to the given file");
    }
  }
  renderer.reset_clipping(true);
  renderer.clear(background);
  pageno++;
}
template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::close() {
  if (pageno == 0) pageno++;
  if (!savePage()) {
    Rf_warning("agg could not write to the given file");
  }
}

template<class PIXFMT, class R_COLOR>
SEXP AggDevice<PIXFMT, R_COLOR>::capture() {
  return Rf_allocVector(INTSXP, 0);
}

/* This takes care of writing the buffer to an appropriate file. The filename
 * may be specified as a printf string with room for a page counter, so the 
 * method should take care of resolving that together with the pageno field.
 */
template<class PIXFMT, class R_COLOR>
bool AggDevice<PIXFMT, R_COLOR>::savePage() {
  return true;
}


// BEHAVIOUR -------------------------------------------------------------------

/* The clipRect method sets clipping on the renderer level, but for performance
 * gain the different drawing methods should set it on the rasterizer as well
 * to avoid unneccesary allocation and looping
 */
template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::clipRect(double x0, double y0, double x1, 
                                          double y1) {
  clip_left = x0;
  clip_right = x1;
  clip_top = y0;
  clip_bottom = y1;
  renderer.clip_box(x0, y0, x1, y1);
}

/* These methods funnel all operations to the text_renderer. See text_renderer.h
 * for implementation details.
 * 
 * They work on gray8 bitmap to speed it up as all metrixs are assumed to be in
 * horizontal mode only
 */
template<class PIXFMT, class R_COLOR>
double AggDevice<PIXFMT, R_COLOR>::stringWidth(const char *str, 
                                               const char *family, int face, 
                                               double size) {
  size *= res_mod;
  agg::glyph_rendering gren = agg::glyph_ren_agg_gray8;
  if (!t_ren.load_font(gren, family, face, size)) {
    return 0.0;
  }
  
  return t_ren.get_text_width(str);
}
template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::charMetric(int c, const char *family, int face, 
                                   double size, double *ascent, double *descent, 
                                   double *width) {
  size *= res_mod;
  agg::glyph_rendering gren = agg::glyph_ren_agg_gray8;
  if (!t_ren.load_font(gren, family, face, size)) {
    *ascent = 0.0;
    *descent = 0.0;
    *width = 0.0;
    return;
  }
  
  t_ren.get_char_metric(c, ascent, descent, width);
}


// DRAWING ---------------------------------------------------------------------

// TODO: Consider using compound rasterizer for drawing fill+stroke so that 
// translucent stroke does not let the fill bleed trough

// TODO: Can the rendering of shapes be refactored to avoid all the code 
// duplication?

/* Draws a circle. Use for standard points as well as grid.circle etc. The 
 * number of points around the circle is precalculated below a radius of 64
 * pixels in order to speed up point rendering
 */
template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::drawCircle(double x, double y, double r, 
                                            int fill, int col, double lwd, 
                                            int lty, R_GE_lineend lend) {
  bool draw_fill = visibleColour(fill);
  bool draw_stroke = visibleColour(col) && lwd > 0.0 && lty != LTY_BLANK;
  
  if (!draw_fill && !draw_stroke) return; // Early exit
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras;
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::scanline_p8 slp;
  agg::ellipse e1;
  if (r < 1) {
    r = r < 0.5 ? 0.5 : r;
    e1.init(x, y, r, r, 4);
  } else if (r < 2.5) {
    e1.init(x, y, r, r, 8);
  } else if (r < 5) {
    e1.init(x, y, r, r, 16);
  } else if (r < 10) {
    e1.init(x, y, r, r, 32);
  } else if (r < 20) {
    e1.init(x, y, r, r, 64);
  } else {
    e1.init(x, y, r, r);
  }
  
  if (draw_fill) {
    ras.add_path(e1);
    agg::render_scanlines_aa_solid(ras, slp, renderer, convertColour(fill));
  }
  if (!draw_stroke) return;
  
  agg::scanline_u8 slu;
  if (lty == LTY_SOLID) {
    agg::conv_stroke<agg::ellipse> pg(e1);
    pg.width(lwd);
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, slu, renderer, convertColour(col));
  } else {
    agg::conv_dash<agg::ellipse> pd(e1);
    agg::conv_stroke< agg::conv_dash<agg::ellipse> > pg(pd);
    makeDash(pd, lty, lwd);
    pg.width(lwd);
    pg.line_cap(convertLineend(lend));
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, slu, renderer, convertColour(col));
  }
}

template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::drawRect(double x0, double y0, double x1, 
                                          double y1, int fill, int col, 
                                          double lwd, int lty, 
                                          R_GE_lineend lend) {
  bool draw_fill = visibleColour(fill);
  bool draw_stroke = visibleColour(col) && lwd > 0.0 && lty != LTY_BLANK;
  
  if (!draw_fill && !draw_stroke) return; // Early exit
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras;
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::scanline_p8 slp;
  agg::path_storage rect;
  rect.remove_all();
  rect.move_to(x0, y0);
  rect.line_to(x0, y1);
  rect.line_to(x1, y1);
  rect.line_to(x1, y0);
  rect.close_polygon();
  
  if (draw_fill) {
    ras.add_path(rect);
    agg::render_scanlines_bin_solid(ras, slp, renderer, convertColour(fill));
  }
  if (!draw_stroke) return;
  
  agg::scanline_u8 slu;
  if (lty == LTY_SOLID) {
    agg::conv_stroke<agg::path_storage> pg(rect);
    pg.width(lwd);
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, slu, renderer, convertColour(col));
  } else {
    agg::conv_dash<agg::path_storage> pd(rect);
    agg::conv_stroke< agg::conv_dash<agg::path_storage> > pg(pd);
    makeDash(pd, lty, lwd);
    pg.width(lwd);
    pg.line_cap(convertLineend(lend));
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, slu, renderer, convertColour(col));
  }
}

template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::drawPolygon(int n, double *x, double *y, 
                                             int fill, int col, double lwd, 
                                             int lty, R_GE_lineend lend, 
                                             R_GE_linejoin ljoin, 
                                             double lmitre) {
  bool draw_fill = visibleColour(fill);
  bool draw_stroke = visibleColour(col) && lwd > 0.0 && lty != LTY_BLANK;
  
  if (n < 2 || (!draw_fill && !draw_stroke)) return; // Early exit
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras;
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::scanline_p8 slp;
  agg::path_storage poly;
  poly.remove_all();
  poly.move_to(x[0], y[0]);
  for (int i = 1; i < n; i++) {
    poly.line_to(x[i], y[i]);
  }
  poly.close_polygon();
  
  if (draw_fill) {
    ras.add_path(poly);
    agg::render_scanlines_aa_solid(ras, slp, renderer, convertColour(fill));
  }
  if (!draw_stroke) return;
  
  agg::scanline_u8 slu;
  if (lty == LTY_SOLID) {
    agg::conv_stroke<agg::path_storage> pg(poly);
    pg.width(lwd);
    pg.line_join(convertLinejoin(ljoin));
    pg.miter_limit(lmitre);
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, slu, renderer, convertColour(col));
  } else {
    agg::conv_dash<agg::path_storage> pd(poly);
    agg::conv_stroke< agg::conv_dash<agg::path_storage> > pg(pd);
    makeDash(pd, lty, lwd);
    pg.width(lwd);
    pg.line_cap(convertLineend(lend));
    pg.line_join(convertLinejoin(ljoin));
    pg.miter_limit(lmitre);
    pg.line_cap(convertLineend(lend));
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, slu, renderer, convertColour(col));
  }
}

template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::drawLine(double x1, double y1, double x2, 
                                          double y2, int col, double lwd, 
                                          int lty, R_GE_lineend lend) {
  if (!visibleColour(col) || lwd == 0.0 || lty == LTY_BLANK) return;
  
  lwd *= lwd_mod;
  
  agg::scanline_u8 sl;
  agg::rasterizer_scanline_aa<> ras;
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::path_storage ps;
  ps.remove_all();
  ps.move_to(x1, y1);
  ps.line_to(x2, y2);
  
  if (lty == LTY_SOLID) {
    agg::conv_stroke<agg::path_storage> pg(ps);
    pg.width(lwd);
    pg.line_cap(convertLineend(lend));
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, sl, renderer, convertColour(col));
  } else {
    agg::conv_dash<agg::path_storage> pd(ps);
    agg::conv_stroke< agg::conv_dash<agg::path_storage> > pg(pd);
    makeDash(pd, lty, lwd);
    pg.width(lwd);
    pg.line_cap(convertLineend(lend));
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, sl, renderer, convertColour(col));
  }
}

template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::drawPolyline(int n, double* x, double* y, 
                                              int col, double lwd, int lty, 
                                              R_GE_lineend lend, 
                                              R_GE_linejoin ljoin, 
                                              double lmitre) {
  if (!visibleColour(col) || lwd == 0.0 || lty == LTY_BLANK || n < 2) return;
  
  lwd *= lwd_mod;
  
  agg::scanline_u8 sl;
  agg::rasterizer_scanline_aa<> ras;
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::path_storage ps;
  ps.remove_all();
  ps.move_to(x[0], y[0]);
  for (int i = 1; i < n; i++) {
    ps.line_to(x[i], y[i]);
  }
  
  if (lty == LTY_SOLID) {
    agg::conv_stroke<agg::path_storage> pg(ps);
    pg.width(lwd);
    pg.line_cap(convertLineend(lend));
    pg.line_join(convertLinejoin(ljoin));
    pg.miter_limit(lmitre);
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, sl, renderer, convertColour(col));
  } else {
    agg::conv_dash<agg::path_storage> pd(ps);
    agg::conv_stroke< agg::conv_dash<agg::path_storage> > pg(pd);
    makeDash(pd, lty, lwd);
    pg.width(lwd);
    pg.line_cap(convertLineend(lend));
    pg.line_join(convertLinejoin(ljoin));
    pg.miter_limit(lmitre);
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, sl, renderer, convertColour(col));
  }
}

template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::drawPath(int npoly, int* nper, double* x, 
                                          double* y, int col, int fill, 
                                          double lwd, int lty, 
                                          R_GE_lineend lend, 
                                          R_GE_linejoin ljoin, double lmitre, 
                                          bool evenodd) {
  bool draw_fill = visibleColour(fill);
  bool draw_stroke = visibleColour(col) && lwd > 0.0 && lty != LTY_BLANK;
  
  if (!draw_fill && !draw_stroke) return; // Early exit
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras;
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::scanline_p8 slp;
  agg::path_storage path;
  path.remove_all();
  int counter = 0;
  for (int i = 0; i < npoly; i++) {
    if (nper[i] < 2) {
      counter += nper[i];
      continue;
    }
    path.move_to(x[counter], y[counter]);
    counter++;
    for (int j = 1; j < nper[i]; j++) {
      path.line_to(x[counter], y[counter]);
      counter++;
    };
    path.close_polygon();
  }
  
  if (draw_fill) {
    ras.add_path(path);
    if (evenodd) ras.filling_rule(agg::fill_even_odd);
    agg::render_scanlines_aa_solid(ras, slp, renderer, convertColour(fill));
  }
  if (!draw_stroke) return;
  
  agg::scanline_u8 slu;
  if (lty == LTY_SOLID) {
    agg::conv_stroke<agg::path_storage> pg(path);
    pg.width(lwd);
    pg.line_join(convertLinejoin(ljoin));
    pg.miter_limit(lmitre);
    ras.filling_rule(agg::fill_non_zero);
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, slu, renderer, convertColour(col));
  } else {
    agg::conv_dash<agg::path_storage> pd(path);
    agg::conv_stroke< agg::conv_dash<agg::path_storage> > pg(pd);
    makeDash(pd, lty, lwd);
    pg.width(lwd);
    pg.line_cap(convertLineend(lend));
    pg.line_join(convertLinejoin(ljoin));
    pg.miter_limit(lmitre);
    pg.line_cap(convertLineend(lend));
    ras.filling_rule(agg::fill_non_zero);
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, slu, renderer, convertColour(col));
  }
}

template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::drawRaster(unsigned int *raster, int w, int h, 
                                            double x, double y, 
                                            double final_width, 
                                            double final_height, double rot, 
                                            bool interpolate) {
  agg::rendering_buffer rbuf(reinterpret_cast<unsigned char*>(raster), w, h, 
                             w * 4);
  
  agg::trans_affine img_mtx;
  img_mtx *= agg::trans_affine_reflection(0);
  img_mtx *= agg::trans_affine_translation(0, h);
  img_mtx *= agg::trans_affine_scaling(final_width / double(w), 
                                       final_height / double (h));
  img_mtx *= agg::trans_affine_rotation(-rot * agg::pi / 180.0);
  img_mtx *= agg::trans_affine_translation(x, y);
  agg::trans_affine src_mtx = img_mtx;
  img_mtx.invert();
  
  typedef agg::span_interpolator_linear<> interpolator_type;
  interpolator_type interpolator(img_mtx);
  
  typedef agg::image_accessor_clone<pixfmt_r_raster> img_source_type;
  
  pixfmt_r_raster img_pixf(rbuf);
  img_source_type img_src(img_pixf);
  agg::span_allocator<agg::rgba8> sa;
  agg::rasterizer_scanline_aa<> ras;
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::scanline_u8 sl;
  
  agg::path_storage rect;
  rect.remove_all();
  rect.move_to(0, 0);
  rect.line_to(0, h);
  rect.line_to(w, h);
  rect.line_to(w, 0);
  rect.close_polygon();
  agg::conv_transform<agg::path_storage> tr(rect, src_mtx);
  ras.add_path(tr);
  
  if (interpolate) {
    typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> span_gen_type;
    span_gen_type sg(img_src, interpolator);
    
    agg::render_scanlines_aa(ras, sl, renderer, sa, sg);
  } else {
    typedef agg::span_image_filter_rgba_nn<img_source_type, interpolator_type> span_gen_type;
    span_gen_type sg(img_src, interpolator);
    
    agg::render_scanlines_aa(ras, sl, renderer, sa, sg);
  }
}

template<class PIXFMT, class R_COLOR>
void AggDevice<PIXFMT, R_COLOR>::drawText(double x, double y, const char *str, 
                                          const char *family, int face, 
                                          double size, double rot, double hadj, 
                                          int col) {
  agg::glyph_rendering gren = std::fmod(rot, 90) == 0.0 ? agg::glyph_ren_agg_gray8 : agg::glyph_ren_outline;
  
  size *= res_mod;
  
  if (!t_ren.load_font(gren, family, face, size)) {
    return;
  }
  
  renderer_solid ren_solid(renderer);
  ren_solid.color(convertColour(col));
  
  t_ren.plot_text(x, y, str, rot, hadj, ren_solid);
}


#endif
