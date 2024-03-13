#pragma once

#include <vector>
#include "ragg.h"
#include "RenderBuffer.h"
#include "rendering.h"

#include "agg_path_storage.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_span_gradient.h"
#include "agg_gradient_lut.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_allocator.h"
#include "agg_scanline_p.h"
#include "agg_span_image_filter_rgba.h"

template<class PIXFMT, class color>
class Group {
public:
  typedef agg::span_interpolator_linear<> interpolator_type;
  typedef agg::span_allocator<color> span_allocator_type;
  
  RenderBuffer<PIXFMT> dst; 
  RenderBuffer<PIXFMT> src; 
  
  span_allocator_type sa;
  int width;
  int height;
  bool clip;
  
  Group(int w, int h, bool must_clip_dst) : dst(), src(), width(w), height(h), clip(must_clip_dst) {
    src.init(clip ? width : 0, clip ? height : 0, color(0, 0, 0, 0));
    dst.init(width, height, color(0, 0, 0, 0));
    
    src.set_comp(agg::comp_op_src_over);
    dst.set_comp(agg::comp_op_src_over);
  }
  
  RenderBuffer<PIXFMT>* buffer() {
    return clip ? &src : &dst;
  }
  
  void do_blend(int MAX_CELLS) {
    if (!clip) return;
    
    agg::rasterizer_scanline_aa<> ras(MAX_CELLS);
    agg::path_storage rect;
    rect.remove_all();
    rect.move_to(0, 0);
    rect.line_to(0, height);
    rect.line_to(width, height);
    rect.line_to(width, 0);
    rect.close_polygon();
    ras.add_path(rect);
    agg::rasterizer_scanline_aa<> ras_clip(MAX_CELLS);
    
    agg::trans_affine mtx;
    
    interpolator_type span_interpolator(mtx);
    PIXFMT img_pixf(src.get_buffer());
    agg::span_allocator<color> sa;
    
    agg::scanline_u8 sl;
    
    typedef agg::image_accessor_clip<PIXFMT> img_source_type;
    img_source_type img_src(img_pixf, color(0, 0, 0, 0));
    
    typedef agg::span_image_filter_rgba_nn<img_source_type, interpolator_type> span_none_type;
    span_none_type span_none(img_src, span_interpolator);
    
    typedef typename RenderBuffer<PIXFMT>::blend_renbase_type ren_type;
    agg::renderer_scanline_aa<ren_type, span_allocator_type, span_none_type> none_renderer(dst.get_renderer_blend(), sa, span_none);
    render<agg::scanline_p8>(ras, ras_clip, sl, none_renderer, false);
    
    src.get_renderer().clear(color(0,0,0,0));
  }
  
  void finish() {
    src.init(0, 0, color(0, 0, 0, 0));
  }
  
  template<class Raster, class RasterClip, class Scanline, class Render>
  void draw(agg::trans_affine mtx, Raster &ras, RasterClip &ras_clip, Scanline &sl, Render &renderer, bool clip) {
    interpolator_type span_interpolator(mtx);
    PIXFMT img_pixf(dst.get_buffer());
    agg::span_allocator<color> sa;
    
    typedef agg::image_accessor_clip<PIXFMT> img_source_type;
    img_source_type img_src(img_pixf, color(0, 0, 0, 0));
    if (mtx.is_identity()) {
      typedef agg::span_image_filter_rgba_nn<img_source_type, interpolator_type> span_none_type;
      span_none_type span_none(img_src, span_interpolator);
      agg::renderer_scanline_aa<Render, span_allocator_type, span_none_type> none_renderer(renderer, sa, span_none);
      render<agg::scanline_p8>(ras, ras_clip, sl, none_renderer, clip);
    } else {
      typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> span_none_type;
      span_none_type span_none(img_src, span_interpolator);
      agg::renderer_scanline_aa<Render, span_allocator_type, span_none_type> none_renderer(renderer, sa, span_none);
      render<agg::scanline_p8>(ras, ras_clip, sl, none_renderer, clip);
    }
  }
  
  Group<pixfmt_type_32, agg::rgba8> convert_for_mask() {
    Group<pixfmt_type_32, agg::rgba8> new_group(width, height, false);
    
    new_group.dst.copy_from<PIXFMT>(dst.get_buffer());
    
    return new_group;
  }
};
