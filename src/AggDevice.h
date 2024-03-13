#pragma once

#include "ragg.h"
#include "rendering.h"
#include "text_renderer.h"
#include "RenderBuffer.h"
#include "pattern.h"
#include "group.h"

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

#include <Rversion.h>

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
  UTF_UCS converter;
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
  bool snap_rect;
  
  double x_trans;
  double y_trans;
  
  TextRenderer<BLNDFMT> t_ren;
  
  // Caches
  std::unordered_map<unsigned int, std::pair<std::unique_ptr<agg::path_storage>, bool> > clip_cache;
  unsigned int clip_cache_next_id;
  agg::path_storage* recording_path;
  agg::path_storage* current_clip;
  bool current_clip_rule_is_evenodd;
  
  std::unordered_map<unsigned int, std::unique_ptr<MaskBuffer> > mask_cache;
  unsigned int mask_cache_next_id;
  MaskBuffer* recording_mask;
  MaskBuffer* current_mask;
  
  std::unordered_map<unsigned int, std::unique_ptr<Pattern<BLNDFMT, R_COLOR> > > pattern_cache;
  unsigned int pattern_cache_next_id;
  std::unordered_map<unsigned int, std::unique_ptr<Group<BLNDFMT, R_COLOR> > > group_cache;
  unsigned int group_cache_next_id;
  RenderBuffer<BLNDFMT>* recording_raster;
  Group<BLNDFMT, R_COLOR>* recording_group;
  
  // Lifecycle methods
  AggDevice(const char* fp, int w, int h, double ps, int bg, double res, 
            double scaling, bool snap);
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
  
  std::unique_ptr<agg::path_storage> recordPath(SEXP path);
  SEXP createClipPath(SEXP path, SEXP ref);
  void removeClipPath(SEXP ref);
  SEXP createMask(SEXP mask, SEXP ref);
  void removeMask(SEXP ref);
  SEXP createPattern(SEXP pattern);
  void removePattern(SEXP ref);
  SEXP renderGroup(SEXP source, int op, SEXP destination);
  void useGroup(SEXP ref, SEXP trans);
  void removeGroup(SEXP ref);
  
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
  void drawGlyph(int n, int *glyphs, double *x, double *y, SEXP font, 
                 double size, int colour, double rot);
  void renderPath(SEXP path, bool do_fill, bool do_stroke, int col, int fill, 
                  double lwd, int lty, R_GE_lineend lend, R_GE_linejoin ljoin, 
                  double lmitre, bool evenodd, int pattern);
  
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
  void setStroke(Raster &ras, Path &p, int lty, double lwd, R_GE_lineend lend, R_GE_linejoin ljoin, double lmitre) {
    if (lty == LTY_SOLID) {
      agg::conv_stroke<Path> pg(p);
      pg.width(lwd);
      pg.line_join(convertLinejoin(ljoin));
      pg.miter_limit(lmitre);
      pg.line_cap(convertLineend(lend));
      ras.add_path(pg);
    } else {
      agg::conv_dash<Path> pd(p);
      agg::conv_stroke< agg::conv_dash<Path> > pg(pd);
      makeDash(pd, lty, lwd);
      pg.width(lwd);
      pg.line_join(convertLinejoin(ljoin));
      pg.miter_limit(lmitre);
      pg.line_cap(convertLineend(lend));
      ras.add_path(pg);
    }
  }
  agg::comp_op_e compositeOperator(int op) {
    agg::comp_op_e comp_op = agg::comp_op_src_over;
#if R_GE_version >= 15
    switch(op) {
    case R_GE_compositeClear: comp_op = agg::comp_op_clear; break;
    case R_GE_compositeSource: comp_op = agg::comp_op_src; break;
    case R_GE_compositeOver: comp_op = agg::comp_op_src_over; break;
    case R_GE_compositeIn: comp_op = agg::comp_op_src_in; break;
    case R_GE_compositeOut: comp_op = agg::comp_op_src_out; break;
    case R_GE_compositeAtop: comp_op = agg::comp_op_src_atop; break;
    case R_GE_compositeDest: comp_op = agg::comp_op_dst; break;
    case R_GE_compositeDestOver: comp_op = agg::comp_op_dst_over; break;
    case R_GE_compositeDestIn: comp_op = agg::comp_op_dst_in; break;
    case R_GE_compositeDestOut: comp_op = agg::comp_op_dst_out; break;
    case R_GE_compositeDestAtop: comp_op = agg::comp_op_dst_atop; break;
    case R_GE_compositeXor: comp_op = agg::comp_op_xor; break;
    case R_GE_compositeAdd: comp_op = agg::comp_op_plus; break;
    case R_GE_compositeSaturate: Rf_warning("`saturate` blending is not supported in ragg"); break;
    case R_GE_compositeMultiply: comp_op = agg::comp_op_multiply; break;
    case R_GE_compositeScreen: comp_op = agg::comp_op_screen; break;
    case R_GE_compositeOverlay: comp_op = agg::comp_op_overlay; break;
    case R_GE_compositeDarken: comp_op = agg::comp_op_darken; break;
    case R_GE_compositeLighten: comp_op = agg::comp_op_lighten; break;
    case R_GE_compositeColorDodge: comp_op = agg::comp_op_color_dodge; break;
    case R_GE_compositeColorBurn: comp_op = agg::comp_op_color_burn; break;
    case R_GE_compositeHardLight: comp_op = agg::comp_op_hard_light; break;
    case R_GE_compositeSoftLight: comp_op = agg::comp_op_soft_light; break;
    case R_GE_compositeDifference: comp_op = agg::comp_op_difference; break;
    case R_GE_compositeExclusion: comp_op = agg::comp_op_exclusion; break;
    }
#endif
    return comp_op;
  }
  bool opClipsSrc(int op) {
    bool clip_src = false;
#if R_GE_version >= 15
    switch(op) {
    case R_GE_compositeSource: 
    case R_GE_compositeIn: 
    case R_GE_compositeOut: 
    case R_GE_compositeDest: 
    case R_GE_compositeDestOver: 
    case R_GE_compositeDestIn: 
    case R_GE_compositeDestAtop: clip_src = true;
    }
#endif
    return clip_src;
  }
  template<class Raster>
  void fillPattern(Raster &ras, Raster &ras_clip, Pattern<BLNDFMT, R_COLOR>& pattern) {
    agg::scanline_u8 sl;
    bool clip = current_clip != NULL;
    if (recording_mask == NULL && recording_raster == NULL) {
      if (current_mask == NULL) {
        pattern.draw(ras, ras_clip, sl, renderer, clip);
      } else {
        if (current_mask->use_luminance()) {
          pattern.draw(ras, ras_clip, current_mask->get_masked_scanline_l(), renderer, clip);
        } else {
          pattern.draw(ras, ras_clip, current_mask->get_masked_scanline_a(), renderer, clip);
        }
      }
    } else if (recording_raster == NULL) {
      Pattern<pixfmt_type_32, agg::rgba8> mask_pattern = pattern.convert_for_mask();
      
      if (current_mask == NULL) {
        mask_pattern.draw(ras, ras_clip, sl, recording_mask->get_renderer(), clip);
      } else {
        if (current_mask->use_luminance()) {
          mask_pattern.draw(ras, ras_clip, current_mask->get_masked_scanline_l(), recording_mask->get_renderer(), clip);
        } else {
          mask_pattern.draw(ras, ras_clip, current_mask->get_masked_scanline_a(), recording_mask->get_renderer(), clip);
        }
      }
    } else {
      if (current_mask == NULL) {
        if (recording_raster->custom_blend) {
          pattern.draw(ras, ras_clip, sl, recording_raster->get_renderer_blend(), clip);
        } else {
          pattern.draw(ras, ras_clip, sl, recording_raster->get_renderer(), clip);
        }
      } else {
        if (recording_raster->custom_blend) {
          if (current_mask->use_luminance()) {
            pattern.draw(ras, ras_clip, current_mask->get_masked_scanline_l(), recording_raster->get_renderer_blend(), clip);
          } else {
            pattern.draw(ras, ras_clip, current_mask->get_masked_scanline_a(), recording_raster->get_renderer_blend(), clip);
          }
        } else {
          if (current_mask->use_luminance()) {
            pattern.draw(ras, ras_clip, current_mask->get_masked_scanline_l(), recording_raster->get_renderer(), clip);
          } else {
            pattern.draw(ras, ras_clip, current_mask->get_masked_scanline_a(), recording_raster->get_renderer(), clip);
          }
        }
      }
      if (recording_group != NULL) {
        recording_group->do_blend(MAX_CELLS);
      }
    }
  }
  template<class Raster, class Path>
  void drawShape(Raster &ras, Raster &ras_clip, Path &path, bool draw_fill, 
                 bool draw_stroke, int fill, int col, double lwd, 
                 int lty, R_GE_lineend lend, R_GE_linejoin ljoin = GE_ROUND_JOIN, 
                 double lmitre = 1.0, int pattern = -1, bool evenodd = false) {
    agg::scanline_p8 slp;
    if (recording_path != NULL) {
      recording_path->concat_path(path);
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
      
      if (recording_mask == NULL && recording_raster == NULL) {
        solid_renderer.color(convertColour(fill));
        if (current_mask == NULL) {
          render<agg::scanline_p8>(ras, ras_clip, slp, solid_renderer, current_clip != NULL);
        } else {
          if (current_mask->use_luminance()) {
            render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline_l(), solid_renderer, current_clip != NULL);
          } else {
            render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline_a(), solid_renderer, current_clip != NULL);
          }
        }
      } else if (recording_raster == NULL) {
        recording_mask->set_colour(convertMaskCol(fill));
        if (current_mask == NULL) {
          render<agg::scanline_p8>(ras, ras_clip, slp, recording_mask->get_solid_renderer(), current_clip != NULL);
        } else {
          if (current_mask->use_luminance()) {
            render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline_l(), recording_mask->get_solid_renderer(), current_clip != NULL);
          } else {
            render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline_a(), recording_mask->get_solid_renderer(), current_clip != NULL);
          }
        }
      } else {
        recording_raster->set_colour(convertColour(fill));
        if (current_mask == NULL) {
          if (recording_raster->custom_blend) {
            render<agg::scanline_p8>(ras, ras_clip, slp, recording_raster->get_solid_renderer_blend(), current_clip != NULL);
          } else {
            render<agg::scanline_p8>(ras, ras_clip, slp, recording_raster->get_solid_renderer(), current_clip != NULL);
          }
        } else {
          if (recording_raster->custom_blend) {
            if (current_mask->use_luminance()) {
              render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline_l(), recording_raster->get_solid_renderer_blend(), current_clip != NULL);
            } else {
              render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline_a(), recording_raster->get_solid_renderer_blend(), current_clip != NULL);
            }
          } else {
            if (current_mask->use_luminance()) {
              render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline_l(), recording_raster->get_solid_renderer(), current_clip != NULL);
            } else {
              render<agg::scanline_p8>(ras, ras_clip, current_mask->get_masked_scanline_a(), recording_raster->get_solid_renderer(), current_clip != NULL);
            }
          }
        }
        if (recording_group != NULL) {
          recording_group->do_blend(MAX_CELLS);
        }
      }
    }
    if (!draw_stroke) return;
    
    if (evenodd) ras.filling_rule(agg::fill_non_zero);
    agg::scanline_u8 slu;
    setStroke(ras, path, lty, lwd, lend, ljoin, lmitre);
    if (recording_mask == NULL && recording_raster == NULL) {
      solid_renderer.color(convertColour(col));
      if (current_mask == NULL) {
        render<agg::scanline_u8>(ras, ras_clip, slu, solid_renderer, current_clip != NULL);
      } else {
        if (current_mask->use_luminance()) {
          render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline_l(), solid_renderer, current_clip != NULL);
        } else {
          render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline_a(), solid_renderer, current_clip != NULL);
        }
      }
    } else if (recording_raster == NULL) {
      recording_mask->set_colour(convertMaskCol(col));
      if (current_mask == NULL) {
        render<agg::scanline_u8>(ras, ras_clip, slu, recording_mask->get_solid_renderer(), current_clip != NULL);
      } else {
        if (current_mask->use_luminance()) {
          render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline_l(), recording_mask->get_solid_renderer(), current_clip != NULL);
        } else {
          render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline_a(), recording_mask->get_solid_renderer(), current_clip != NULL);
        }
      }
    } else {
      recording_raster->set_colour(convertColour(col));
      if (current_mask == NULL) {
        if (recording_raster->custom_blend) {
          render<agg::scanline_u8>(ras, ras_clip, slu, recording_raster->get_solid_renderer_blend(), current_clip != NULL);
        } else {
          render<agg::scanline_u8>(ras, ras_clip, slu, recording_raster->get_solid_renderer(), current_clip != NULL);
        }
      } else {
        if (recording_raster->custom_blend) {
          if (current_mask->use_luminance()) {
            render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline_l(), recording_raster->get_solid_renderer_blend(), current_clip != NULL);
          } else {
            render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline_a(), recording_raster->get_solid_renderer_blend(), current_clip != NULL);
          }
        } else {
          if (current_mask->use_luminance()) {
            render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline_l(), recording_raster->get_solid_renderer(), current_clip != NULL);
          } else {
            render<agg::scanline_u8>(ras, ras_clip, current_mask->get_masked_scanline_a(), recording_raster->get_solid_renderer(), current_clip != NULL);
          }
        }
      }
      if (recording_group != NULL) {
        recording_group->do_blend(MAX_CELLS);
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
                                               int bg, double res, double scaling, 
                                               bool snap) : 
  converter(),
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
  snap_rect(snap),
  x_trans(0.0),
  y_trans(0.0),
  t_ren(),
  clip_cache_next_id(0),
  recording_path(NULL),
  current_clip(NULL),
  current_clip_rule_is_evenodd(false),
  mask_cache_next_id(0),
  recording_mask(NULL),
  current_mask(NULL),
  pattern_cache_next_id(0),
  recording_raster(NULL),
  recording_group(NULL)
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
  if (recording_raster != NULL && x0 == 0.0 && y0 == height && x1 == width && y1 == 0.0) {
    // resetting clipping while recording a pattern
    // I hate this heuristic
    clip_left = 0.0;
    clip_right = recording_raster->width;
    clip_top = 0.0;
    clip_bottom = recording_raster->height;
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
#if R_VERSION >= R_Version(4, 0, 0)
  if (face == 5) {
    const char* str2 = Rf_utf8Toutf8NoPUA(str);
    str = str2;
  }
#endif
  
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
#if R_VERSION >= R_Version(4, 0, 0)
    if (face == 5) {
      char str[16];
      Rf_ucstoutf8(str, (unsigned int) c);
      const char* str2 = Rf_utf8Toutf8NoPUA(str);
      int n = 0;
      uint32_t* res = converter.convert(str2, n);
      if (n > 0) c = res[0];
    }
#endif
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
std::unique_ptr<agg::path_storage> AggDevice<PIXFMT, R_COLOR, BLNDFMT>::recordPath(SEXP path) {
  std::unique_ptr<agg::path_storage> new_path(new agg::path_storage());
  
  // Assign container pointer to device
  recording_path = new_path.get();
  
  SEXP R_fcall = PROTECT(Rf_lang1(path));
  Rf_eval(R_fcall, R_GlobalEnv);
  UNPROTECT(1);
  
  recording_path = NULL;
  
  return new_path;
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
    std::unique_ptr<agg::path_storage> new_clip = recordPath(path);
    
    current_clip = new_clip.get();
    current_clip_rule_is_evenodd = false;
    
#if R_GE_version >= 15
    current_clip_rule_is_evenodd = R_GE_clipPathFillRule(path) == R_GE_evenOddRule;
#endif
    
    clip_cache[key] = {std::move(new_clip), current_clip_rule_is_evenodd};
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
    bool luminance = false;
#if R_GE_version >= 15
    luminance = R_GE_maskType(mask) == R_GE_luminanceMask;
#endif
    new_mask->init(width, height, luminance);
    
    // Assign container pointer to device
    MaskBuffer* temp_mask = recording_mask;
    RenderBuffer<BLNDFMT>* temp_raster = recording_raster;
    recording_mask = new_mask.get();
    recording_raster = NULL;
    
    SEXP R_fcall = PROTECT(Rf_lang1(mask));
    Rf_eval(R_fcall, R_GlobalEnv);
    UNPROTECT(1);
    
    current_mask = recording_mask;
    recording_raster = temp_raster;
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
    RenderBuffer<BLNDFMT>* temp_raster = recording_raster;
    
    x_trans += new_pattern->x_trans;
    y_trans += new_pattern->y_trans;
    clip_left = 0.0;
    clip_right = R_GE_tilingPatternWidth(pattern);
    clip_top = 0.0;
    clip_bottom = R_GE_tilingPatternHeight(pattern);
    if (clip_bottom < 0) clip_bottom = -clip_bottom;
    recording_mask = NULL;
    current_mask = NULL;
    recording_raster = &(new_pattern->buffer);
    
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
    recording_raster = temp_raster;
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

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
SEXP AggDevice<PIXFMT, R_COLOR, BLNDFMT>::renderGroup(SEXP source, int op, SEXP destination) {
  if (Rf_isNull(source)) {
    return Rf_ScalarInteger(-1);
  }
  int key = group_cache_next_id;
  group_cache_next_id++;
  
  std::unique_ptr<Group<BLNDFMT, R_COLOR> > new_group(new Group<BLNDFMT, R_COLOR>(width, height, opClipsSrc(op) && destination != R_NilValue));
  
  double temp_clip_left = clip_left;
  double temp_clip_right = clip_right;
  double temp_clip_top = clip_top;
  double temp_clip_bottom = clip_bottom;
  
  MaskBuffer* temp_mask = recording_mask;
  MaskBuffer* temp_current_mask = current_mask;
  Group<BLNDFMT, R_COLOR>* temp_group = recording_group;
  RenderBuffer<BLNDFMT>* temp_raster = recording_raster;
  
  clip_left = 0.0;
  clip_right = width;
  clip_top = 0.0;
  clip_bottom = height;
  recording_mask = NULL;
  current_mask = NULL;
  recording_group = NULL;
  recording_raster = &(new_group->dst);
  
  if (destination != R_NilValue) {
    SEXP R_fcall = PROTECT(Rf_lang1(destination));
    Rf_eval(R_fcall, R_GlobalEnv);
    UNPROTECT(1);
  }
  
  recording_raster->set_comp(compositeOperator(op));
  
  recording_raster = new_group->buffer();
  recording_group = new_group.get();
  
  SEXP R_fcall = PROTECT(Rf_lang1(source));
  Rf_eval(R_fcall, R_GlobalEnv);
  UNPROTECT(1);
  
  new_group->finish();
  
  clip_left = temp_clip_left;
  clip_right = temp_clip_right;
  clip_top = temp_clip_top;
  clip_bottom = temp_clip_bottom;
  
  recording_mask = temp_mask;
  current_mask = temp_current_mask;
  recording_group = temp_group;
  recording_raster = temp_raster;
  
  group_cache[key] = std::move(new_group);
  
  return Rf_ScalarInteger(key);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::useGroup(SEXP ref, SEXP trans) {
  int key = INTEGER(ref)[0];
  if (key < 0) {
    Rf_warning("Unknown group");
    return;
  }
  auto it = group_cache.find(key);
  if (it == group_cache.end()) {
    Rf_warning("Unknown group");
    return;
  }
  agg::trans_affine mtx;
  
  if (trans != R_NilValue) {
    mtx = agg::trans_affine(
      REAL(trans)[0],
      REAL(trans)[3],
      REAL(trans)[1],
      REAL(trans)[4],
      REAL(trans)[2],
      REAL(trans)[5]
    );
    mtx.invert();
  }
  
  bool clip = current_clip != NULL;
  
  agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  if (clip) {
    ras_clip.add_path(*current_clip);
    if (current_clip_rule_is_evenodd) {
      ras_clip.filling_rule(agg::fill_even_odd);
    }
  }
  
  agg::path_storage rect;
  rect.remove_all();
  rect.move_to(0, 0);
  rect.line_to(0, height);
  rect.line_to(width, height);
  rect.line_to(width, 0);
  rect.close_polygon();
  ras.add_path(rect);
  
  agg::scanline_u8 sl;
  if (recording_mask == NULL && recording_raster == NULL) {
    if (current_mask == NULL) {
      it->second->draw(mtx, ras, ras_clip, sl, renderer, clip);
    } else {
      if (current_mask->use_luminance()) {
        it->second->draw(mtx, ras, ras_clip, current_mask->get_masked_scanline_l(), renderer, clip);
      } else {
        it->second->draw(mtx, ras, ras_clip, current_mask->get_masked_scanline_a(), renderer, clip);
      }
    }
  } else if (recording_raster == NULL) {
    Group<pixfmt_type_32, agg::rgba8> mask_group = it->second->convert_for_mask();
    
    if (current_mask == NULL) {
      mask_group.draw(mtx, ras, ras_clip, sl, recording_mask->get_renderer(), clip);
    } else {
      if (current_mask->use_luminance()) {
        mask_group.draw(mtx, ras, ras_clip, current_mask->get_masked_scanline_l(), recording_mask->get_renderer(), clip);
      } else {
        mask_group.draw(mtx, ras, ras_clip, current_mask->get_masked_scanline_a(), recording_mask->get_renderer(), clip);
      }
    }
  } else {
    if (current_mask == NULL) {
      if (recording_raster->custom_blend) {
        it->second->draw(mtx, ras, ras_clip, sl, recording_raster->get_renderer_blend(), clip);
      } else {
        it->second->draw(mtx, ras, ras_clip, sl, recording_raster->get_renderer(), clip);
      }
    } else {
      if (recording_raster->custom_blend) {
        if (current_mask->use_luminance()) {
          it->second->draw(mtx, ras, ras_clip, current_mask->get_masked_scanline_l(), recording_raster->get_renderer_blend(), clip);
        } else {
          it->second->draw(mtx, ras, ras_clip, current_mask->get_masked_scanline_a(), recording_raster->get_renderer_blend(), clip);
        }
      } else {
        if (current_mask->use_luminance()) {
          it->second->draw(mtx, ras, ras_clip, current_mask->get_masked_scanline_l(), recording_raster->get_renderer(), clip);
        } else {
          it->second->draw(mtx, ras, ras_clip, current_mask->get_masked_scanline_a(), recording_raster->get_renderer(), clip);
        }
      }
    }
    if (recording_group != NULL) {
      recording_group->do_blend(MAX_CELLS);
    }
  }
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::removeGroup(SEXP ref) {
  if (Rf_isNull(ref)) {
    group_cache.clear();
    group_cache_next_id = 0;
    return;
  }
  
  unsigned int key = INTEGER(ref)[0];
  auto it = group_cache.find(key);
  // Check if path exists
  if (it != group_cache.end()) {
    group_cache.erase(it);
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
  
  drawShape(ras, ras_clip, e1, draw_fill, draw_stroke, fill, col, lwd, lty, lend, GE_ROUND_JOIN, 1.0, pattern);
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
  if (snap_rect && draw_fill && !draw_stroke) {
    x0 = std::round(x0);
    x1 = std::round(x1);
    y0 = std::round(y0);
    y1 = std::round(y1);
  }
  rect.remove_all();
  rect.move_to(x0, y0);
  rect.line_to(x0, y1);
  rect.line_to(x1, y1);
  rect.line_to(x1, y0);
  rect.close_polygon();
  
  drawShape(ras, ras_clip, rect, draw_fill, draw_stroke, fill, col, lwd, lty, lend, GE_ROUND_JOIN, 1.0, pattern);
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
  
  drawShape(ras, ras_clip, poly, draw_fill, draw_stroke, fill, col, lwd, lty, lend, ljoin, lmitre, pattern);
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
  
  drawShape(ras, ras_clip, ps, false, true, 0, col, lwd, lty, lend, ljoin, lmitre);
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
  
  drawShape(ras, ras_clip, path, draw_fill, draw_stroke, fill, col, lwd, lty, lend, ljoin, lmitre, pattern, evenodd);
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::renderPath(SEXP path, bool do_fill, bool do_stroke, 
                                                     int col, int fill, 
                                                     double lwd, int lty, 
                                                     R_GE_lineend lend, 
                                                     R_GE_linejoin ljoin, double lmitre, 
                                                     bool evenodd, int pattern) {
  bool draw_fill = do_fill && (visibleColour(fill) || pattern != -1);
  bool draw_stroke = do_stroke && (visibleColour(col) && lwd > 0.0 && lty != LTY_BLANK);
  
  if (!draw_fill && !draw_stroke) return; // Early exit
  
  lwd *= lwd_mod;
  
  agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
  agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
  ras.clip_box(clip_left, clip_top, clip_right, clip_bottom);
  std::unique_ptr<agg::path_storage> recorded_path = recordPath(path);
  
  drawShape(ras, ras_clip, *recorded_path, draw_fill, draw_stroke, fill, col, lwd, lty, lend, ljoin, lmitre, pattern, evenodd);
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
  if (recording_mask == NULL && recording_raster == NULL) {
    if (current_mask == NULL) {
      render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, slu, interpolator, renderer, interpolate, current_clip != NULL, false);
    } else {
      if (current_mask->use_luminance()) {
        render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline_l(), interpolator, renderer, interpolate, current_clip != NULL, false);
      } else {
        render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline_a(), interpolator, renderer, interpolate, current_clip != NULL, false);
      }
    }
  } else if (recording_raster == NULL) {
    if (current_mask == NULL) {
      render_raster<pixfmt_r_raster, pixfmt_type_32>(rbuf, w, h, ras, ras_clip, slu, interpolator, recording_mask->get_renderer(), interpolate, current_clip != NULL, false);
    } else {
      if (current_mask->use_luminance()) {
        render_raster<pixfmt_r_raster, pixfmt_type_32>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline_l(), interpolator, recording_mask->get_renderer(), interpolate, current_clip != NULL, false);
      } else {
        render_raster<pixfmt_r_raster, pixfmt_type_32>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline_a(), interpolator, recording_mask->get_renderer(), interpolate, current_clip != NULL, false);
      }
    }
  } else {
    if (current_mask == NULL) {
      if (recording_raster->custom_blend) {
        render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, slu, interpolator, recording_raster->get_renderer_blend(), interpolate, current_clip != NULL, false);
      } else {
        render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, slu, interpolator, recording_raster->get_renderer(), interpolate, current_clip != NULL, false);
      }
    } else {
      if (recording_raster->custom_blend) {
        if (current_mask->use_luminance()) {
          render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline_l(), interpolator, recording_raster->get_renderer_blend(), interpolate, current_clip != NULL, false);
        } else {
          render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline_a(), interpolator, recording_raster->get_renderer_blend(), interpolate, current_clip != NULL, false);
        }
      } else {
        if (current_mask->use_luminance()) {
          render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline_l(), interpolator, recording_raster->get_renderer(), interpolate, current_clip != NULL, false);
        } else {
          render_raster<pixfmt_r_raster, BLNDFMT>(rbuf, w, h, ras, ras_clip, current_mask->get_masked_scanline_a(), interpolator, recording_raster->get_renderer(), interpolate, current_clip != NULL, false);
        }
      }
    }
    if (recording_group != NULL) {
      recording_group->do_blend(MAX_CELLS);
    }
  }
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawText(double x, double y, const char *str, 
                                          const char *family, int face, 
                                          double size, double rot, double hadj, 
                                          int col) {
#if R_VERSION >= R_Version(4, 0, 0)
  if (face == 5) {
    const char* str2 = Rf_utf8Toutf8NoPUA(str);
    str = str2;
  }
#endif
  
  agg::glyph_rendering gren = std::fmod(rot, 90) == 0.0 && recording_path == NULL ? agg::glyph_ren_agg_gray8 : agg::glyph_ren_outline;
  
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
  if (recording_mask == NULL && recording_raster == NULL) {
    solid_renderer.color(convertColour(col));
    if (current_mask == NULL) {
      t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, solid_renderer, renderer, slu, device_id, ras_clip, current_clip != NULL, recording_path);
    } else {
      if (current_mask->use_luminance()) {
        t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, solid_renderer, renderer, current_mask->get_masked_scanline_l(), device_id, ras_clip, current_clip != NULL, recording_path);
      } else {
        t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, solid_renderer, renderer, current_mask->get_masked_scanline_a(), device_id, ras_clip, current_clip != NULL, recording_path);
      }
    }
  } else if (recording_raster == NULL) {
    recording_mask->set_colour(convertMaskCol(col));
    if (current_mask == NULL) {
      t_ren.template plot_text<pixfmt_type_32>(x, y, str, rot, hadj, recording_mask->get_solid_renderer(), recording_mask->get_renderer(), slu, device_id, ras_clip, current_clip != NULL, recording_path);
    } else {
      if (current_mask->use_luminance()) {
        t_ren.template plot_text<pixfmt_type_32>(x, y, str, rot, hadj, recording_mask->get_solid_renderer(), recording_mask->get_renderer(), current_mask->get_masked_scanline_l(), device_id, ras_clip, current_clip != NULL, recording_path);
      } else {
        t_ren.template plot_text<pixfmt_type_32>(x, y, str, rot, hadj, recording_mask->get_solid_renderer(), recording_mask->get_renderer(), current_mask->get_masked_scanline_a(), device_id, ras_clip, current_clip != NULL, recording_path);
      }
    }
  } else {
    recording_raster->set_colour(convertColour(col));
    if (current_mask == NULL) {
      if (recording_raster->custom_blend) {
        t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, recording_raster->get_solid_renderer(), recording_raster->get_renderer_blend(), slu, device_id, ras_clip, current_clip != NULL, recording_path);
      } else {
        t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, recording_raster->get_solid_renderer(), recording_raster->get_renderer(), slu, device_id, ras_clip, current_clip != NULL, recording_path);
      }
    } else {
      if (recording_raster->custom_blend) {
        if (current_mask->use_luminance()) {
          t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, recording_raster->get_solid_renderer(), recording_raster->get_renderer_blend(), current_mask->get_masked_scanline_l(), device_id, ras_clip, current_clip != NULL, recording_path);
        } else {
          t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, recording_raster->get_solid_renderer(), recording_raster->get_renderer_blend(), current_mask->get_masked_scanline_a(), device_id, ras_clip, current_clip != NULL, recording_path);
        }
      } else {
        if (current_mask->use_luminance()) {
          t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, recording_raster->get_solid_renderer(), recording_raster->get_renderer(), current_mask->get_masked_scanline_l(), device_id, ras_clip, current_clip != NULL, recording_path);
        } else {
          t_ren.template plot_text<BLNDFMT>(x, y, str, rot, hadj, recording_raster->get_solid_renderer(), recording_raster->get_renderer(), current_mask->get_masked_scanline_a(), device_id, ras_clip, current_clip != NULL, recording_path);
        }
      }
    }
    if (recording_group != NULL) {
      recording_group->do_blend(MAX_CELLS);
    }
  }
}

template<class PIXFMT, class R_COLOR, typename BLNDFMT>
void AggDevice<PIXFMT, R_COLOR, BLNDFMT>::drawGlyph(int n, int *glyphs, 
                                                    double *x, double *y, 
                                                    SEXP font, double size,
                                                    int colour, double rot) {
  agg::glyph_rendering gren = std::fmod(rot, 90) == 0.0 && recording_path == NULL ? agg::glyph_ren_agg_gray8 : agg::glyph_ren_outline;
  
  int i;
  for (i=0; i<n; i++) {
    *x += x_trans;
    *y += y_trans;
  }
  
  size *= res_mod;
  
  // Start by finding with family to pre-populate font feature settings
  FontSettings font_info;
  
#if R_GE_version >= 16
  strncpy(font_info.file, R_GE_glyphFontFile(font), PATH_MAX);
  font_info.index = R_GE_glyphFontIndex(font);
  font_info.features = NULL;
  font_info.n_features = 0;
#endif
  
  if (!t_ren.load_font_from_file(font_info, gren, size, device_id)) {
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
  if (recording_mask == NULL && recording_raster == NULL) {
    solid_renderer.color(convertColour(colour));
    if (current_mask == NULL) {
      t_ren.template plot_glyphs<BLNDFMT>(n, glyphs, x, y, rot, solid_renderer, renderer, slu, ras_clip, current_clip != NULL, recording_path);
    } else {
      if (current_mask->use_luminance()) {
        t_ren.template plot_glyphs<BLNDFMT>(n, glyphs, x, y, rot, solid_renderer, renderer, current_mask->get_masked_scanline_l(), ras_clip, current_clip != NULL, recording_path);
      } else {
        t_ren.template plot_glyphs<BLNDFMT>(n, glyphs, x, y, rot, solid_renderer, renderer, current_mask->get_masked_scanline_a(), ras_clip, current_clip != NULL, recording_path);
      }
    }
  } else if (recording_raster == NULL) {
    recording_mask->set_colour(convertMaskCol(colour));
    if (current_mask == NULL) {
      t_ren.template plot_glyphs<pixfmt_type_32>(n, glyphs, x, y, rot, recording_mask->get_solid_renderer(), recording_mask->get_renderer(), slu, ras_clip, current_clip != NULL, recording_path);
    } else {
      if (current_mask->use_luminance()) {
        t_ren.template plot_glyphs<pixfmt_type_32>(n, glyphs, x, y, rot, recording_mask->get_solid_renderer(), recording_mask->get_renderer(), current_mask->get_masked_scanline_l(), ras_clip, current_clip != NULL, recording_path);
      } else {
        t_ren.template plot_glyphs<pixfmt_type_32>(n, glyphs, x, y, rot, recording_mask->get_solid_renderer(), recording_mask->get_renderer(), current_mask->get_masked_scanline_a(), ras_clip, current_clip != NULL, recording_path);
      }
    }
  } else {
    recording_raster->set_colour(convertColour(colour));
    if (current_mask == NULL) {
      if (recording_raster->custom_blend) {
        t_ren.template plot_glyphs<BLNDFMT>(n, glyphs, x, y, rot, recording_raster->get_solid_renderer(), recording_raster->get_renderer_blend(), slu, ras_clip, current_clip != NULL, recording_path);
      } else {
        t_ren.template plot_glyphs<BLNDFMT>(n, glyphs, x, y, rot, recording_raster->get_solid_renderer(), recording_raster->get_renderer(), slu, ras_clip, current_clip != NULL, recording_path);
      }
    } else {
      if (recording_raster->custom_blend) {
        if (current_mask->use_luminance()) {
          t_ren.template plot_glyphs<BLNDFMT>(n, glyphs, x, y, rot, recording_raster->get_solid_renderer(), recording_raster->get_renderer_blend(), current_mask->get_masked_scanline_l(), ras_clip, current_clip != NULL, recording_path);
        } else {
          t_ren.template plot_glyphs<BLNDFMT>(n, glyphs, x, y, rot, recording_raster->get_solid_renderer(), recording_raster->get_renderer_blend(), current_mask->get_masked_scanline_a(), ras_clip, current_clip != NULL, recording_path);
        }
      } else {
        if (current_mask->use_luminance()) {
          t_ren.template plot_glyphs<BLNDFMT>(n, glyphs, x, y, rot, recording_raster->get_solid_renderer(), recording_raster->get_renderer(), current_mask->get_masked_scanline_l(), ras_clip, current_clip != NULL, recording_path);
        } else {
          t_ren.template plot_glyphs<BLNDFMT>(n, glyphs, x, y, rot, recording_raster->get_solid_renderer(), recording_raster->get_renderer(), current_mask->get_masked_scanline_a(), ras_clip, current_clip != NULL, recording_path);
        }
      }
    }
    if (recording_group != NULL) {
      recording_group->do_blend(MAX_CELLS);
    }
  }
}
