#pragma once

#include "ragg.h"
#include "rendering.h"
#include "text_renderer.h"
#include "RenderBuffer.h"
#include "pattern.h"

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
#include "agg_scanline_boolean_algebra.h"

#include "util/agg_color_conv.h"

#include <memory>

static const int MAX_CELLS = 1 << 20;

/* Base class for graphic device interface to AGG. See AggDevice.cpp for 
 * implementation details. 
 * 
 * Specific devices should subclass this and provide their own buffer and 
 * savePage() methods (at least), while the drawing methods should work 
 * regardless. The base class outputs images in ppm format which is not realy
 * a usable format. See png and tiff versions for actual usable classes.
 */
template<class PIXFMT, class R_COLOR = agg::rgba8, typename BLNDFMT = pixfmt_type_32>
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
  unsigned int device_id;
  
  renbase_type renderer;
  renderer_solid solid_renderer;
  pixfmt_type* pixf;
  agg::rendering_buffer rbuf;
  unsigned char* buffer;
  
  int pageno;
  std::string file;
  R_COLOR background;
  int background_int;
  double pointsize;
  double res_real;
  double res_mod;
  double lwd_mod;
  
  double x_trans;
  double y_trans;
  
  TextRenderer<BLNDFMT> t_ren;
  
  // Caches
  std::unordered_map<unsigned int, std::pair<std::unique_ptr<agg::path_storage>, bool> > clip_cache;
  unsigned int clip_cache_next_id;
  agg::path_storage* recording_clip;
  agg::path_storage* current_clip;
  bool current_clip_rule_is_evenodd;
  
  std::unordered_map<unsigned int, std::unique_ptr<MaskBuffer> > mask_cache;
  unsigned int mask_cache_next_id;
  MaskBuffer* recording_mask;
  MaskBuffer* current_mask;
  
  std::unordered_map<unsigned int, std::unique_ptr<Pattern<BLNDFMT, R_COLOR> > > pattern_cache;
  unsigned int pattern_cache_next_id;
  RenderBuffer<BLNDFMT>* recording_pattern;
  
  // Lifecycle methods
  AggDevice(const char* fp, int w, int h, double ps, int bg, double res, 
            double scaling);
  virtual ~AggDevice();
  virtual void newPage(unsigned int bg);
  void close();
  virtual bool savePage();
  SEXP capture();
  
  // Behaviour
  void clipRect(double x0, double y0, double x1, double y1);
  double stringWidth(const char *str, const char *family, int face, 
                     double size);
  void charMetric(int c, const char *family, int face, double size,
                  double *ascent, double *descent, double *width);
  SEXP createClipPath(SEXP path, SEXP ref);
  void removeClipPath(SEXP ref);
  SEXP createMask(SEXP mask, SEXP ref);
  void removeMask(SEXP ref);
  SEXP createPattern(SEXP pattern);
  void removePattern(SEXP ref);
  
  // Drawing Methods
  void drawCircle(double x, double y, double r, int fill, int col, double lwd, 
                  int lty, R_GE_lineend lend, int pattern);
  void drawRect(double x0, double y0, double x1, double y1, int fill, int col, 
                double lwd, int lty, R_GE_lineend lend, int pattern);
  void drawPolygon(int n, double *x, double *y, int fill, int col, double lwd, 
                   int lty, R_GE_lineend lend, R_GE_linejoin ljoin, 
                   double lmitre, int pattern);
  void drawLine(double x1, double y1, double x2, double y2, int col, double lwd, 
                int lty, R_GE_lineend lend);
  void drawPolyline(int n, double* x, double* y, int col, double lwd, int lty,
                    R_GE_lineend lend, R_GE_linejoin ljoin, double lmitre);
  void drawPath(int npoly, int* nper, double* x, double* y, int col, int fill, 
                double lwd, int lty, R_GE_lineend lend, R_GE_linejoin ljoin, 
                double lmitre, bool evenodd, int pattern);
  void drawRaster(unsigned int *raster, int w, int h, double x, double y, 
                  double final_width, double final_height, double rot, 
                  bool interpolate);
  void drawText(double x, double y, const char *str, const char *family, 
                int face, double size, double rot, double hadj, int col);
  void typesetText(SEXP span, double x, double y, double w);
  void drawGlyph(SEXP glyph, double x, double y);
  
protected:
  virtual inline R_COLOR convertColour(unsigned int col) {
    return R_COLOR(R_RED(col), R_GREEN(col), R_BLUE(col), R_ALPHA(col)).premultiply();
  }
  virtual inline agg::rgba32 convertMaskCol(unsigned int col) {
    return agg::rgba32(R_COLOR(R_RED(col), R_GREEN(col), R_BLUE(col), R_ALPHA(col))).premultiply();
  }
  inline bool visibleColour(unsigned int col) {
    return (int) !R_TRANSPARENT(col);
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
  template<class Raster, class Path>
  void setStroke(Raster &ras, Path &p, int lty, double lwd, R_GE_lineend lend, R_GE_linejoin ljoin) {
    if (lty == LTY_SOLID) {
      agg::conv_stroke<Path> pg(p);
      pg.width(lwd);
      pg.line_join(convertLinejoin(ljoin));
      pg.line_cap(convertLineend(lend));
      ras.add_path(pg);
    } else {
      agg::conv_dash<Path> pd(p);
      agg::conv_stroke< agg::conv_dash<Path> > pg(pd);
      makeDash(pd, lty, lwd);
      pg.width(lwd);
      pg.line_join(convertLinejoin(ljoin));
      pg.line_cap(convertLineend(lend));
      ras.add_path(pg);
    }
  }
  template<class Raster>
  void fillPattern(Raster &ras, Raster &ras_clip, Pattern<BLNDFMT, R_COLOR>& pattern) {
    agg::scanline_u8 sl;
    bool clip = current_clip != NULL;
    if (recording_mask == NULL && recording_pattern == NULL) {
      if (current_mask == NULL) {
        pattern.draw(ras, ras_clip, sl, renderer, clip);
      } else {
        pattern.draw(ras, ras_clip, current_mask->get_masked_scanline(), renderer, clip);
      }
    } else if (recording_pattern == NULL) {
      Pattern<pixfmt_type_32, agg::rgba8> mask_pattern = pattern.convert_for_mask();
      
      if (current_mask == NULL) {
        mask_pattern.draw(ras, ras_clip, sl, recording_mask->get_renderer(), clip);
      } else {
        mask_pattern.draw(ras, ras_clip, current_mask->get_masked_scanline(), recording_mask->get_renderer(), clip);
      }
    } else {
      if (current_mask == NULL) {
        pattern.draw(ras, ras_clip, sl, recording_pattern->get_renderer(), clip);
      } else {
        pattern.draw(ras, ras_clip, current_mask->get_masked_scanline(), recording_pattern->get_renderer(), clip);
      }
    }
  }
  template<class Raster, class Path>
  void drawShape(Raster &ras, Raster &ras_clip, Path &path, bool draw_fill, 
                 bool draw_stroke, int fill, int col, double lwd, 
                 int lty, R_GE_lineend lend, R_GE_linejoin ljoin = GE_ROUND_JOIN, int pattern = -1, bool evenodd = false) {
    agg::scanline_p8 slp;
    if (recording_clip != NULL) {
      recording_clip->concat_path(path);
      return;
    }
    if (current_clip != NULL) {
      ras_clip.add_path(*current_clip);
      if (current_clip_rule_is_evenodd) {
        ras_clip.filling_rule(agg::fill_even_odd);
      }
    }
    
    if (pattern != -1) {
      ras.add_path(path);
      if (evenodd) ras.filling_rule(agg::fill_even_odd);
      
      auto pat_it = pattern_cache.find(pattern);
      if (pat_it != pattern_cache.end()) {
        fillPattern(ras, ras_clip, *(pat_it->second));
      }
    } else if (draw_fill) {
      ras.add_path(path);
      if (evenodd) ras.filling_rule(agg::fill_even_odd);
      
      if (recording_mask == NULL && recording_pattern == NULL) {
        solid_renderer.color(convertColour(fill));
        if (current_mask == NULL) {
          render<agg::scanline_p8>(ras, ras_clip, slp, solid_renderer, current_clip != NULL);
        } else {
          render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline(), solid_renderer, current_clip != NULL);
        }
      } else if (recording_pattern == NULL) {
        recording_mask->set_colour(convertMaskCol(fill));
        if (current_mask == NULL) {
          render<agg::scanline_p8>(ras, ras_clip, slp, recording_mask->get_solid_renderer(), current_clip != NULL);
        } else {
          render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline(), recording_mask->get_solid_renderer(), current_clip != NULL);
        }
      } else {
        recording_pattern->set_colour(convertColour(fill));
        if (current_mask == NULL) {
          render<agg::scanline_p8>(ras, ras_clip, slp, recording_pattern->get_solid_renderer(), current_clip != NULL);
        } else {
          render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline(), recording_pattern->get_solid_renderer(), current_clip != NULL);
        }
      }
    }
    if (!draw_stroke) return;
    
    if (evenodd) ras.filling_rule(agg::fill_non_zero);
    agg::scanline_u8 slu;
    setStroke(ras, path, lty, lwd, lend, ljoin);
    if (recording_mask == NULL && recording_pattern == NULL) {
      solid_renderer.color(convertColour(col));
      if (current_mask == NULL) {
        render<agg::scanline_u8>(ras, ras_clip, slu, solid_renderer, current_clip != NULL);
      } else {
        render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline(), solid_renderer, current_clip != NULL);
      }
    } else if (recording_pattern == NULL) {
      recording_mask->set_colour(convertMaskCol(col));
      if (current_mask == NULL) {
        render<agg::scanline_u8>(ras, ras_clip, slu, recording_mask->get_solid_renderer(), current_clip != NULL);
      } else {
        render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline(), recording_mask->get_solid_renderer(), current_clip != NULL);
      }
    } else {
      recording_pattern->set_colour(convertMaskCol(col));
      if (current_mask == NULL) {
        render<agg::scanline_u8>(ras, ras_clip, slu, recording_pattern->get_solid_renderer(), current_clip != NULL);
      } else {
        render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline(), recording_pattern->get_solid_renderer(), current_clip != NULL);
      }
    }
  }
};

// IMPLIMENTATION --------------------------------------------------------------

// LIFECYCLE -------------------------------------------------------------------

/* The initialiser takes care of setting up the buffer, and caching a pixel 
 * formatter and renderer.
 */
template<class PIXFMT, class R_COLOR, typename BLNDFMT>
AggDevice<PIXFMT, R_COLOR, BLNDFMT>::AggDevice(const char* fp, int w, int h, double ps, 
                                               int bg, double res, double scaling) : 
  width(w),
  height(h),
  clip_left(0),
  clip_right(w),
  clip_top(0),
  clip_bottom(h),
  device_id(0),
  pageno(0),
  file(fp),
  background_int(bg),
  pointsize(ps),
  res_real(res),
  res_mod(scaling * res / 72.0),
  lwd_mod(scaling * res / 96.0),
  x_trans(0.0),
  y_trans(0.0),
  t_ren(),
  clip_cache_next_id(0),
  recording_clip(NULL),
  current_clip(NULL),
  current_clip_rule_is_evenodd(false),
  mask_cache_next_id(0),
  recording_mask(NULL),
  current_mask(NULL),
  pattern_cache_next_id(0),
  recording_pattern(NULL)
{
  buffer = new unsigned char[width * height * bytes_per_pixel];
  rbuf = agg::rendering_buffer(buffer, width, height, width * bytes_per_pixel);
  pixf = new pixfmt_type(rbuf);
  renderer = renbase_type(*pixf);
  solid_renderer = renderer_solid(renderer);
  background = convertColour(background_int);
  renderer.clear(background);
}
template<class PIXFMT, class R_COLOR, typename BLNDFMT>
AggDevice<PIXFMT, R_COLOR, BLNDFMT>::~AggDevice() {
  delete pixf;
  delete [] buffer;
}

/* newPage() should not need to be overwritten as long the class have an 
 * appropriate savePage() method. For screen devices it may make sense to change
 * it for performance
 */
template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::newPage(unsigned int bg) {
  if (pageno != 0) {
    if (!savePage()) {
      Rf_warning("agg could not write to the given file");
    }
  }
  renderer.reset_clipping(true);
  if (visibleColour(bg)) {
    renderer.clear(convertColour(bg));
  } else {
    renderer.clear(background);
  }
  pageno++;
}
template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::close() {
  if (pageno == 0) pageno++;
  if (!savePage()) {
    Rf_warning("agg could not write to the given file");
  }
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
SEXP AggDevice<PIXFMT, R_COLOR, BLNDFMT>::capture() {
  return Rf_allocVector(INTSXP, 0);
}

/* This takes care of writing the buffer to an appropriate file. The filename
 * may be specified as a printf string with room for a page counter, so the 
 * method should take care of resolving that together with the pageno field.
 */
template<class PIXFMT, class R_COLOR, typename BLNDFMT>
bool AggDevice<PIXFMT, R_COLOR, BLNDFMT>::savePage() {
  return true;
}


// BEHAVIOUR -------------------------------------------------------------------

/* The clipRect method sets clipping on the renderer level, but for performance
 * gain the different drawing methods should set it on the rasterizer as well
 * to avoid unneccesary allocation and looping
 */
template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::clipRect(double x0, double y0, double x1, 
                                          double y1) {
  if (recording_pattern != NULL && x0 == 0.0 && y0 == height && x1 == width && y1 == 0.0) {
    // resetting clipping while recording a pattern
    // I hate this heuristic
    clip_left = 0.0;
    clip_right = recording_pattern->width;
    clip_top = 0.0;
    clip_bottom = recording_pattern->height;
    return;
  }
  clip_left = x0 + x_trans;
  clip_right = x1 + x_trans;
  clip_top = y0 + y_trans;
  clip_bottom = y1 + y_trans;
  renderer.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  current_clip = NULL;
  current_clip_rule_is_evenodd = false;
}

/* These methods funnel all operations to the text_renderer. See text_renderer.h
 * for implementation details.
 * 
 * They work on gray8 bitmap to speed it up as all metrixs are assumed to be in
 * horizontal mode only
 */
template<class PIXFMT, class R_COLOR, typename BLNDFMT>
double AggDevice<PIXFMT, R_COLOR, BLNDFMT>::stringWidth(const char *str, 
                                               const char *family, int face, 
                                               double size) {
  size *= res_mod;
  agg::glyph_rendering gren = agg::glyph_ren_agg_gray8;
  if (!t_ren.load_font(gren, family, face, size, device_id)) {
    return 0.0;
  }
  
  return t_ren.get_text_width(str);
}
template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::charMetric(int c, const char *family, int face, 
                                   double size, double *ascent, double *descent, 
                                   double *width) {
  if (c < 0) {
    c = -c;
  }
  
  size *= res_mod;
  agg::glyph_rendering gren = agg::glyph_ren_agg_gray8;
  if (!t_ren.load_font(gren, family, face, size, device_id)) {
    *ascent = 0.0;
    *descent = 0.0;
    *width = 0.0;
    return;
  }
  
  t_ren.get_char_metric(c, ascent, descent, width);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
SEXP AggDevice<PIXFMT, R_COLOR, BLNDFMT>::createClipPath(SEXP path, SEXP ref) {
  int key;
  if (Rf_isNull(path)) {
    return Rf_ScalarInteger(-1);
  }
  if (Rf_isNull(ref)) {
    key = clip_cache_next_id;
    clip_cache_next_id++;
  } else {
    key = INTEGER(ref)[0];
    if (key < 0) {
      return Rf_ScalarInteger(key);
    }
  }
  
  auto clip_cache_iter = clip_cache.find(key);
  // Check if path exists
  if (clip_cache_iter == clip_cache.end()) {
    // Path doesn't exist - create a new entry and get reference to it
    std::unique_ptr<agg::path_storage> new_clip(new agg::path_storage());
    
    bool new_clip_is_even_odd = false;
#if R_GE_version >= 15
    new_clip_is_even_odd = R_GE_clipPathFillRule(path) == R_GE_evenOddRule;
#endif
    
    // Assign container pointer to device
    recording_clip = new_clip.get();
    
    SEXP R_fcall = PROTECT(Rf_lang1(path));
    Rf_eval(R_fcall, R_GlobalEnv);
    UNPROTECT(1);
    current_clip = recording_clip;
    current_clip_rule_is_evenodd = new_clip_is_even_odd;
    
    recording_clip = NULL;
    
    clip_cache[key] = {std::move(new_clip), new_clip_is_even_odd};
  } else {
    current_clip = clip_cache_iter->second.first.get();
    current_clip_rule_is_evenodd = clip_cache_iter->second.second;
  }
  clip_left = 0.0;
  clip_right = width;
  clip_top = 0.0;
  clip_bottom = height;
  renderer.reset_clipping(true);
  
  return Rf_ScalarInteger(key);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::removeClipPath(SEXP ref) {
  if (Rf_isNull(ref)) {
    clip_cache.clear();
    clip_cache_next_id = 0;
    return;
  }
    
  int key = INTEGER(ref)[0];
  
  if (key < 0) {
    return;
  }
  
  auto it = clip_cache.find(key);
  // Check if path exists
  if (it != clip_cache.end()) {
    clip_cache.erase(it);
  }
  
  return;
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
SEXP AggDevice<PIXFMT, R_COLOR, BLNDFMT>::createMask(SEXP mask, SEXP ref) {
  int key;
  if (Rf_isNull(mask)) {
    current_mask = NULL;
    return Rf_ScalarInteger(-1);
  }
  if (Rf_isNull(ref)) {
    key = mask_cache_next_id;
    mask_cache_next_id++;
  } else {
    key = INTEGER(ref)[0];
    if (key < 0) {
      current_mask = NULL;
      return Rf_ScalarInteger(key);
    }
  }
  
  auto mask_cache_iter = mask_cache.find(key);
  // Check if path exists
  if (mask_cache_iter == mask_cache.end()) {
    // Mask doesn't exist - create a new entry and get reference to it
    std::unique_ptr<MaskBuffer> new_mask(new MaskBuffer());
    new_mask->init(width, height);
    
    // Assign container pointer to device
    MaskBuffer* temp_mask = recording_mask;
    RenderBuffer<BLNDFMT>* temp_pattern = recording_pattern;
    recording_mask = new_mask.get();
    recording_pattern = NULL;
    
    SEXP R_fcall = PROTECT(Rf_lang1(mask));
    Rf_eval(R_fcall, R_GlobalEnv);
    UNPROTECT(1);
    
    current_mask = recording_mask;
    recording_pattern = temp_pattern;
    recording_mask = temp_mask;
    
    mask_cache[key] = std::move(new_mask);
    
  } else {
    current_mask = mask_cache_iter->second.get();
  }
  
  return Rf_ScalarInteger(key);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::removeMask(SEXP ref) {
  if (Rf_isNull(ref)) {
    mask_cache.clear();
    mask_cache_next_id = 0;
    return;
  }
  
  unsigned int key = INTEGER(ref)[0];
  
  auto it = mask_cache.find(key);
  // Check if path exists
  if (it != mask_cache.end()) {
    mask_cache.erase(it);
  }
  
  return;
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
SEXP AggDevice<PIXFMT, R_COLOR, BLNDFMT>::createPattern(SEXP pattern) {
  if (Rf_isNull(pattern)) {
    return Rf_ScalarInteger(-1);
  }
  int key = pattern_cache_next_id;
  pattern_cache_next_id++;
  
  std::unique_ptr<Pattern<BLNDFMT, R_COLOR> > new_pattern(new Pattern<BLNDFMT, R_COLOR>());
  
#if R_GE_version >= 13
  ExtendType extend = ExtendNone;
  
  switch(R_GE_patternType(pattern)) {
  case R_GE_linearGradientPattern: 
    switch(R_GE_linearGradientExtend(pattern)) {
    case R_GE_patternExtendNone: extend = ExtendNone; break;
    case R_GE_patternExtendPad: extend = ExtendPad; break;
    case R_GE_patternExtendReflect: extend = ExtendReflect; break;
    case R_GE_patternExtendRepeat: extend = ExtendRepeat; break;
    }
    new_pattern->init_linear(R_GE_linearGradientX1(pattern) + x_trans,
                             R_GE_linearGradientY1(pattern) + y_trans,
                             R_GE_linearGradientX2(pattern) + x_trans,
                             R_GE_linearGradientY2(pattern) + y_trans,
                             extend);
    for (int i = 0; i < R_GE_linearGradientNumStops(pattern); ++i) {
      R_COLOR col = convertColour(R_GE_linearGradientColour(pattern, i));
      double stop = R_GE_linearGradientStop(pattern, i);
      new_pattern->add_color(stop, col);
    }
    new_pattern->finish_gradient();
    break;
  case R_GE_radialGradientPattern:
    switch(R_GE_radialGradientExtend(pattern)) {
    case R_GE_patternExtendNone: extend = ExtendNone; break;
    case R_GE_patternExtendPad: extend = ExtendPad; break;
    case R_GE_patternExtendReflect: extend = ExtendReflect; break;
    case R_GE_patternExtendRepeat: extend = ExtendRepeat; break;
    }
    new_pattern->init_radial(R_GE_radialGradientCX1(pattern) + x_trans,
                             R_GE_radialGradientCY1(pattern) + y_trans,
                             R_GE_radialGradientR1(pattern),
                             R_GE_radialGradientCX2(pattern) + x_trans,
                             R_GE_radialGradientCY2(pattern) + y_trans,
                             R_GE_radialGradientR2(pattern),
                             extend);
    for (int i = 0; i < R_GE_radialGradientNumStops(pattern); ++i) {
      R_COLOR col = convertColour(R_GE_radialGradientColour(pattern, i));
      double stop = R_GE_radialGradientStop(pattern, i);
      new_pattern->add_color(stop, col);
    }
    new_pattern->finish_gradient();
    break;
  case R_GE_tilingPattern:
    switch(R_GE_tilingPatternExtend(pattern)) {
    case R_GE_patternExtendNone: extend = ExtendNone; break;
    case R_GE_patternExtendPad: extend = ExtendPad; break;
    case R_GE_patternExtendReflect: extend = ExtendReflect; break;
    case R_GE_patternExtendRepeat: extend = ExtendRepeat; break;
    }
    new_pattern->init_tile(R_GE_tilingPatternWidth(pattern), 
                           R_GE_tilingPatternHeight(pattern), 
                           R_GE_tilingPatternX(pattern) + x_trans, 
                           R_GE_tilingPatternY(pattern) + y_trans, 
                           extend);
    
    double temp_clip_left = clip_left;
    double temp_clip_right = clip_right;
    double temp_clip_top = clip_top;
    double temp_clip_bottom = clip_bottom;
    
    MaskBuffer* temp_mask = recording_mask;
    MaskBuffer* temp_current_mask = current_mask;
    RenderBuffer<BLNDFMT>* temp_pattern = recording_pattern;
    
    x_trans += new_pattern->x_trans;
    y_trans += new_pattern->y_trans;
    clip_left = 0.0;
    clip_right = R_GE_tilingPatternWidth(pattern);
    clip_top = 0.0;
    clip_bottom = R_GE_tilingPatternHeight(pattern);
    if (clip_bottom < 0) clip_bottom = -clip_bottom;
    recording_mask = NULL;
    current_mask = NULL;
    recording_pattern = &(new_pattern->buffer);
    
    SEXP R_fcall = PROTECT(Rf_lang1(R_GE_tilingPatternFunction(pattern)));
    Rf_eval(R_fcall, R_GlobalEnv);
    UNPROTECT(1);
    
    clip_left = temp_clip_left;
    clip_right = temp_clip_right;
    clip_top = temp_clip_top;
    clip_bottom = temp_clip_bottom;
    
    x_trans -= new_pattern->x_trans;
    y_trans -= new_pattern->y_trans;
    recording_mask = temp_mask;
    current_mask = temp_current_mask;
    recording_pattern = temp_pattern;
    break;
  }
#endif
  
  pattern_cache[key] = std::move(new_pattern);
  
  return Rf_ScalarInteger(key);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::removePattern(SEXP ref) {
  if (Rf_isNull(ref)) {
    pattern_cache.clear();
    pattern_cache_next_id = 0;
    return;
  }
  
  unsigned int key = INTEGER(ref)[0];
  auto it = pattern_cache.find(key);
  // Check if path exists
  if (it != pattern_cache.end()) {
    pattern_cache.erase(it);
  }
  
  return;
}

// DRAWING ---------------------------------------------------------------------

/* Draws a circle. Use for standard points as well as grid.circle etc. The 
 * number of points around the circle is precalculated below a radius of 64
 * pixels in order to speed up point rendering
 */
template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawCircle(double x, double y, double r, 
                                            int fill, int col, double lwd, 
                                            int lty, R_GE_lineend lend, int pattern) {
  bool draw_fill = visibleColour(fill) || pattern != -1;
  bool draw_stroke = visibleColour(col) && lwd > 0.0 && lty != LTY_BLANK;
  
  if (!draw_fill && !draw_stroke) return; // Early exit
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::ellipse e1;
  x += x_trans;
  y += y_trans;
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
  
  drawShape(ras, ras_clip, e1, draw_fill, draw_stroke, fill, col, lwd, lty, lend, GE_ROUND_JOIN, pattern);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawRect(double x0, double y0, double x1, 
                                          double y1, int fill, int col, 
                                          double lwd, int lty, 
                                          R_GE_lineend lend, int pattern) {
  bool draw_fill = visibleColour(fill) || pattern != -1;
  bool draw_stroke = visibleColour(col) && lwd > 0.0 && lty != LTY_BLANK;
  
  if (!draw_fill && !draw_stroke) return; // Early exit
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::path_storage rect;
  x0 += x_trans;
  x1 += x_trans;
  y0 += y_trans;
  y1 += y_trans;
  rect.remove_all();
  rect.move_to(x0, y0);
  rect.line_to(x0, y1);
  rect.line_to(x1, y1);
  rect.line_to(x1, y0);
  rect.close_polygon();
  
  drawShape(ras, ras_clip, rect, draw_fill, draw_stroke, fill, col, lwd, lty, lend, GE_ROUND_JOIN, pattern);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawPolygon(int n, double *x, double *y, 
                                             int fill, int col, double lwd, 
                                             int lty, R_GE_lineend lend, 
                                             R_GE_linejoin ljoin, 
                                             double lmitre, int pattern) {
  bool draw_fill = visibleColour(fill) || pattern != -1;
  bool draw_stroke = visibleColour(col) && lwd > 0.0 && lty != LTY_BLANK;
  
  if (n < 2 || (!draw_fill && !draw_stroke)) return; // Early exit
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::path_storage poly;
  poly.remove_all();
  poly.move_to(x[0] + x_trans, y[0] + y_trans);
  for (int i = 1; i < n; i++) {
    poly.line_to(x[i] + x_trans, y[i] + y_trans);
  }
  poly.close_polygon();
  
  drawShape(ras, ras_clip, poly, draw_fill, draw_stroke, fill, col, lwd, lty, lend, ljoin, pattern);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawLine(double x1, double y1, double x2, 
                                          double y2, int col, double lwd, 
                                          int lty, R_GE_lineend lend) {
  if (!visibleColour(col) || lwd == 0.0 || lty == LTY_BLANK) return;
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::path_storage ps;
  ps.remove_all();
  ps.move_to(x1 + x_trans, y1 + y_trans);
  ps.line_to(x2 + x_trans, y2 + y_trans);
  
  drawShape(ras, ras_clip, ps, false, true, 0, col, lwd, lty, lend);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawPolyline(int n, double* x, double* y, 
                                              int col, double lwd, int lty, 
                                              R_GE_lineend lend, 
                                              R_GE_linejoin ljoin, 
                                              double lmitre) {
  if (!visibleColour(col) || lwd == 0.0 || lty == LTY_BLANK || n < 2) return;
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::path_storage ps;
  ps.remove_all();
  ps.move_to(x[0]  + x_trans, y[0] + y_trans);
  for (int i = 1; i < n; i++) {
    ps.line_to(x[i]  + x_trans, y[i] + y_trans);
  }
  
  drawShape(ras, ras_clip, ps, false, true, 0, col, lwd, lty, lend, ljoin);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawPath(int npoly, int* nper, double* x, 
                                          double* y, int col, int fill, 
                                          double lwd, int lty, 
                                          R_GE_lineend lend, 
                                          R_GE_linejoin ljoin, double lmitre, 
                                          bool evenodd, int pattern) {
  bool draw_fill = visibleColour(fill) || pattern != -1;
  bool draw_stroke = visibleColour(col) && lwd > 0.0 && lty != LTY_BLANK;
  
  if (!draw_fill && !draw_stroke) return; // Early exit
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::scanline_p8 slp, slp_clip, slp_result;
  agg::path_storage path;
  path.remove_all();
  int counter = 0;
  for (int i = 0; i < npoly; i++) {
    if (nper[i] < 2) {
      counter += nper[i];
      continue;
    }
    path.move_to(x[counter] + x_trans, y[counter]  + y_trans);
    counter++;
    for (int j = 1; j < nper[i]; j++) {
      path.line_to(x[counter] + x_trans, y[counter]  + y_trans);
      counter++;
    };
    path.close_polygon();
  }
  
  drawShape(ras, ras_clip, path, draw_fill, draw_stroke, fill, col, lwd, lty, lend, ljoin, pattern, evenodd);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawRaster(unsigned int *raster, int w, int h, 
                                            double x, double y, 
                                            double final_width, 
                                            double final_height, double rot, 
                                            bool interpolate) {
  agg::rendering_buffer rbuf(reinterpret_cast<unsigned char*>(raster), w, h, 
                             w * 4);
  
  x += x_trans;
  y += y_trans;

  double x_scale = final_width / double(w);
  double y_scale = final_height / double (h);
  
  agg::trans_affine img_mtx;
  img_mtx *= agg::trans_affine_reflection(0);
  img_mtx *= agg::trans_affine_translation(0, h);
  img_mtx *= agg::trans_affine_scaling(x_scale, y_scale);
  img_mtx *= agg::trans_affine_rotation(-rot * agg::pi / 180.0);
  img_mtx *= agg::trans_affine_translation(x, y);
  agg::trans_affine src_mtx = img_mtx;
  img_mtx.invert();
  
  typedef agg::span_interpolator_linear<> interpolator_type;
  interpolator_type interpolator(img_mtx);
  
  agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  if (current_clip != NULL) {
    ras_clip.add_path(*current_clip);
    if (current_clip_rule_is_evenodd) {
      ras_clip.filling_rule(agg::fill_even_odd);
    }
  }
  
  agg::path_storage rect;
  rect.remove_all();
  rect.move_to(0, 0);
  rect.line_to(0, h);
  rect.line_to(w, h);
  rect.line_to(w, 0);
  rect.close_polygon();
  agg::conv_transform<agg::path_storage> tr(rect, src_mtx);
  ras.add_path(tr);
  
  agg::scanline_u8 slu;
  if (recording_mask == NULL && recording_pattern == NULL) {
    if (current_mask == NULL) {
      render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, slu, interpolator, renderer, interpolate, current_clip != NULL, false);
    } else{
      render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline(), interpolator, renderer, interpolate, current_clip != NULL, false);
    }
  } else if (recording_pattern == NULL) {
    if (current_mask == NULL) {
      render_raster<pixfmt_r_raster, pixfmt_type_32>(rbuf, w, h, ras, ras_clip, slu, interpolator, recording_mask->get_renderer(), interpolate, current_clip != NULL, false);
    } else {
      render_raster<pixfmt_r_raster, pixfmt_type_32>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline(), interpolator, recording_mask->get_renderer(), interpolate, current_clip != NULL, false);
    }
  } else {
    if (current_mask == NULL) {
      render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, slu, interpolator, recording_pattern->get_renderer(), interpolate, current_clip != NULL, false);
    } else {
      render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline(), interpolator, recording_pattern->get_renderer(), interpolate, current_clip != NULL, false);
    }
  }
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawText(double x, double y, const char *str, 
                                          const char *family, int face, 
                                          double size, double rot, double hadj, 
                                          int col) {
  agg::glyph_rendering gren = std::fmod(rot, 90) == 0.0 && recording_clip == NULL ? agg::glyph_ren_agg_gray8 : agg::glyph_ren_outline;
  
  x += x_trans;
  y += y_trans;
  
  size *= res_mod;
  
  if (!t_ren.load_font(gren, family, face, size, device_id)) {
    return;
  }
  
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  if (current_clip != NULL) {
    ras_clip.add_path(*current_clip);
    if (current_clip_rule_is_evenodd) {
      ras_clip.filling_rule(agg::fill_even_odd);
    }
  }
  
  agg::scanline_u8 slu;
  if (recording_mask == NULL && recording_pattern == NULL) {
    solid_renderer.color(convertColour(col));
    if (current_mask == NULL) {
      t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, solid_renderer, renderer, slu, device_id, ras_clip, current_clip != NULL, recording_clip);
    } else {
      t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, solid_renderer, renderer, current_mask->get_masked_scanline(), device_id, ras_clip, current_clip != NULL, recording_clip);
    }
  } else if (recording_pattern == NULL) {
    recording_mask->set_colour(convertMaskCol(col));
    if (current_mask == NULL) {
      t_ren.template plot_text<pixfmt_type_32>(x, y, str, rot, hadj, recording_mask->get_solid_renderer(), recording_mask->get_renderer(), slu, device_id, ras_clip, current_clip != NULL, recording_clip);
    } else {
      t_ren.template plot_text<pixfmt_type_32>(x, y, str, rot, hadj, recording_mask->get_solid_renderer(), recording_mask->get_renderer(), current_mask->get_masked_scanline(), device_id, ras_clip, current_clip != NULL, recording_clip);
    }
  } else {
    recording_pattern->set_colour(convertColour(col));
    if (current_mask == NULL) {
      t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, recording_pattern->get_solid_renderer(), recording_pattern->get_renderer(), slu, device_id, ras_clip, current_clip != NULL, recording_clip);
    } else {
      t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, recording_pattern->get_solid_renderer(), recording_pattern->get_renderer(), current_mask->get_masked_scanline(), device_id, ras_clip, current_clip != NULL, recording_clip);
    }
  }
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::typesetText(SEXP span, 
                                                      double x, double y,
                                                      double w) {

  double size = 12.0;
  int col = R_GE_str2col("black");

  /* 'gren' and 't_ren.load_font()' shifted to layout_text() 
   * because there may be more than one font
   */

  x += x_trans;
  y += y_trans;
  
  size *= res_mod;

  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  if (current_clip != NULL) {
    ras_clip.add_path(*current_clip);
    if (current_clip_rule_is_evenodd) {
      ras_clip.filling_rule(agg::fill_even_odd);
    }
  }
  
  agg::scanline_u8 slu;
  if (recording_mask == NULL && recording_pattern == NULL) {
    solid_renderer.color(convertColour(col));
    if (current_mask == NULL) {
      t_ren.template layout_text<BLNDFMT>(x, y, span, w,
                                          solid_renderer, renderer, 
                                          slu, device_id, 
                                          ras_clip, current_clip != NULL,
                                          recording_clip);
    } else {
      t_ren.template layout_text<BLNDFMT>(x, y, span, w,
                                          solid_renderer, renderer, 
                                          current_mask->get_masked_scanline(), 
                                          device_id, 
                                          ras_clip, current_clip != NULL,
                                          recording_clip);
    }
  } else if (recording_pattern == NULL) {
    recording_mask->set_colour(convertMaskCol(col));
    if (current_mask == NULL) {
      t_ren.template layout_text<pixfmt_type_32>(x, y, span, w,
                                                 recording_mask->get_solid_renderer(), 
                                                 recording_mask->get_renderer(),
                                                 slu, device_id, 
                                                 ras_clip, current_clip != NULL,
                                                 recording_clip);
    } else {
      t_ren.template layout_text<pixfmt_type_32>(x, y, span, w,
                                                 recording_mask->get_solid_renderer(), 
                                                 recording_mask->get_renderer(),
                                                 current_mask->get_masked_scanline(), 
                                                 device_id, 
                                                 ras_clip, current_clip != NULL,
                                                 recording_clip);
    }
  } else {
    recording_pattern->set_colour(convertColour(col));
    if (current_mask == NULL) {
      t_ren.template layout_text<BLNDFMT>(x, y, span, w,
                                          recording_pattern->get_solid_renderer(), 
                                          recording_pattern->get_renderer(), 
                                          slu, device_id, 
                                          ras_clip, current_clip != NULL,
                                          recording_clip);
    } else {
      t_ren.template layout_text<BLNDFMT>(x, y, span, w,
                                          recording_pattern->get_solid_renderer(), 
                                          recording_pattern->get_renderer(), 
                                          current_mask->get_masked_scanline(), 
                                          device_id, 
                                          ras_clip, current_clip != NULL,
                                          recording_clip);
    }
  }
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawGlyph(SEXP glyph, 
                                                    double x, double y) {
  double size = 12.0;
  int col = R_GE_str2col("black");

  /* 'gren' and 't_ren.load_font()' shifted to layout_text() 
   * because there may be more than one font
   */

  x += x_trans;
  y += y_trans;
  
  size *= res_mod;

  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  if (current_clip != NULL) {
    ras_clip.add_path(*current_clip);
    if (current_clip_rule_is_evenodd) {
      ras_clip.filling_rule(agg::fill_even_odd);
    }
  }
  
  agg::scanline_u8 slu;
  if (recording_mask == NULL && recording_pattern == NULL) {
    solid_renderer.color(convertColour(col));
    if (current_mask == NULL) {
      t_ren.template render_glyphs<BLNDFMT>(x, y, glyph, 
                                            solid_renderer, renderer, 
                                            slu, device_id, 
                                            ras_clip, current_clip != NULL,
                                            recording_clip);
    } else {
      t_ren.template render_glyphs<BLNDFMT>(x, y, glyph, 
                                            solid_renderer, renderer, 
                                            current_mask->get_masked_scanline(),
                                            device_id, 
                                            ras_clip, current_clip != NULL,
                                            recording_clip);
    }
  } else if (recording_pattern == NULL) {
    recording_mask->set_colour(convertMaskCol(col));
    if (current_mask == NULL) {
      t_ren.template render_glyphs<pixfmt_type_32>(x, y, glyph, 
                                                   recording_mask->get_solid_renderer(), 
                                                   recording_mask->get_renderer(),
                                                   slu, device_id, 
                                                   ras_clip, 
                                                   current_clip != NULL,
                                                   recording_clip);
    } else {
      t_ren.template render_glyphs<pixfmt_type_32>(x, y, glyph, 
                                                   recording_mask->get_solid_renderer(), 
                                                   recording_mask->get_renderer(),
                                                   current_mask->get_masked_scanline(), 
                                                   device_id, 
                                                   ras_clip, 
                                                   current_clip != NULL,
                                                   recording_clip);
    }
  } else {
    recording_pattern->set_colour(convertColour(col));
    if (current_mask == NULL) {
      t_ren.template render_glyphs<BLNDFMT>(x, y, glyph, 
                                            recording_pattern->get_solid_renderer(), 
                                            recording_pattern->get_renderer(), 
                                            slu, device_id, 
                                            ras_clip, current_clip != NULL,
                                            recording_clip);
    } else {
      t_ren.template render_glyphs<BLNDFMT>(x, y, glyph, 
                                            recording_pattern->get_solid_renderer(), 
                                            recording_pattern->get_renderer(), 
                                            current_mask->get_masked_scanline(), 
                                            device_id, 
                                            ras_clip, current_clip != NULL,
                                            recording_clip);
    }
  }

}
