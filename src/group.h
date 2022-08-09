#pragma once

#include <vector>
#include "ragg.h"
#include "RenderBuffer.h"
#include "rendering.h"

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
  
  RenderBuffer<PIXFMT> buffer; 
  
  span_allocator_type sa;
  int width;
  int height;
  
  Group(int w, int h) : buffer(), width(w), height(h) {
    buffer.init(width, height, color(0, 0, 0, 0));
  }
  
  template<class Raster, class RasterClip, class Scanline, class Render>
  void draw(agg::trans_affine mtx, Raster &ras, RasterClip &ras_clip, Scanline &sl, Render &renderer, bool clip) {
    interpolator_type span_interpolator(mtx);
    PIXFMT img_pixf(buffer.get_buffer());
    agg::span_allocator<color> sa;
    
    typedef agg::image_accessor_clip<PIXFMT> img_source_type;
    img_source_type img_src(img_pixf, color(0, 0, 0, 0));
    typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> span_none_type;
    span_none_type span_none(img_src, span_interpolator);
    agg::renderer_scanline_aa<Render, span_allocator_type, span_none_type> none_renderer(renderer, sa, span_none);
    render<agg::scanline_p8>(ras, ras_clip, sl, none_renderer, clip);
  }
  
  Group<pixfmt_type_32, agg::rgba8> convert_for_mask() {
    Group<pixfmt_type_32, agg::rgba8> new_group(width, height);
    
    new_group.buffer.copy_from<PIXFMT>(buffer.get_buffer());
    
    return new_group;
  }
};
