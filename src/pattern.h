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

enum PatternType {
  PatternLinearGradient,
  PatternRadialGradient,
  PatternTile
};
enum ExtendType {
  ExtendPad,
  ExtendRepeat,
  ExtendReflect,
  ExtendNone
};

template<class PIXFMT, class color>
class Pattern {
public:
  typedef agg::gradient_x linear_type;
  typedef agg::gradient_radial_focus radial_type;
  typedef agg::span_interpolator_linear<> interpolator_type;
  typedef agg::span_allocator<color> span_allocator_type;
  typedef agg::gradient_lut< agg::color_interpolator<color>, 512 > color_func_type;
  typedef agg::gradient_lut< agg::color_interpolator<agg::rgba8>, 512 > color_func_type_mask;
  
  PatternType type;
  ExtendType extend;
  RenderBuffer<PIXFMT> buffer; 
  
  color_func_type gradient;
  color_func_type_mask gradient_mask;
  linear_type linear;
  radial_type radial;
  span_allocator_type sa;
  double d2;
  int width;
  int height;
  
  agg::trans_affine mtx;
  double x_trans;
  double y_trans;
  
  Pattern() : buffer(), d2(0.0), width(0), height(0), x_trans(0.0), y_trans(0.0) {
    gradient.remove_all();
    gradient_mask.remove_all();
    mtx.reset();
  }
  
  void init_linear(double x1, double y1, double x2, double y2, ExtendType e) {
    type = PatternLinearGradient;
    extend = e;
    double dx = x2 - x1;
    double dy = y2 - y1;
    d2 = std::sqrt(dx * dx + dy * dy);
    mtx *= agg::trans_affine_rotation(atan2(dy, dx));
    mtx *= agg::trans_affine_translation(x1, y1);
    mtx.invert();
  }
  
  void init_radial(double x1, double y1, double r1, double x2, double y2, double r2, ExtendType e) {
    type = PatternRadialGradient;
    extend = e;
    d2 = r2 + r1;
    mtx *= agg::trans_affine_translation(x2, y2);
    mtx.invert();
    radial.init(r2, x1 - x2, y1 - y2);
  }
  
  void init_tile(int w, int h, double x, double y, ExtendType e) {
    type = PatternTile;
    extend = e;
    width = w < 0 ? -w : w;
    height = h < 0 ? -h : h;
    buffer.init(width, height, color(0, 0, 0, 0));
    mtx *= agg::trans_affine_translation(0, h);
    mtx *= agg::trans_affine_translation(x, y);
    mtx.invert();
    x_trans = -x;
    y_trans = -y + height;
  }
  
  bool is_gradient() {
    return type != PatternTile;
  }
  
  void add_color(double at, color &col) {
    gradient.add_color(at, col);
    gradient_mask.add_color(at, col);
  }
  
  void finish_gradient() {
    gradient.build_lut();
    gradient_mask.build_lut();
  }
  
  template<class Raster, class RasterClip, class Scanline, class Render>
  void draw(Raster &ras, RasterClip &ras_clip, Scanline &sl, Render &renderer, bool clip) {
    switch (type) {
    case PatternLinearGradient: draw_linear(ras, ras_clip, sl, renderer, clip); break;
    case PatternRadialGradient: draw_radial(ras, ras_clip, sl, renderer, clip); break;
    case PatternTile: draw_tile(ras, ras_clip, sl, renderer, clip); break;
    }
  }
  
  Pattern<pixfmt_type_32, agg::rgba8> convert_for_mask() {
    Pattern<pixfmt_type_32, agg::rgba8> new_pattern;
    
    if (type == PatternTile) {
      new_pattern.init_tile(width, height, 0, 0, extend);
      new_pattern.buffer.copy_from<PIXFMT>(buffer.get_buffer());
    } else{
      new_pattern.type = type;
      new_pattern.extend = extend;
      new_pattern.d2 = d2;
      new_pattern.radial = radial;
      new_pattern.gradient = gradient_mask;
    }
    new_pattern.mtx = mtx;
    
    return new_pattern;
  }
  
private:
  template<class Raster, class RasterClip, class Scanline, class Render>
  void draw_linear(Raster &ras, RasterClip &ras_clip, Scanline &sl, Render &renderer, bool clip) {
    interpolator_type span_interpolator(mtx);
    switch (extend) {
    case ExtendReflect: {
      agg::gradient_reflect_adaptor<linear_type> grad_refl(linear);
      typedef agg::span_gradient<color, interpolator_type, agg::gradient_reflect_adaptor<linear_type>, color_func_type> span_reflect_type;
      
      span_reflect_type span_reflect(span_interpolator, 
                                     grad_refl, 
                                     gradient, 
                                     0, d2);
      
      typedef agg::renderer_scanline_aa<Render, span_allocator_type, span_reflect_type> renderer_reflect_type;
      renderer_reflect_type reflect_renderer(renderer, sa, span_reflect);
      
      render<agg::scanline_p8>(ras, ras_clip, sl, reflect_renderer, clip);
      break;
    }
    case ExtendRepeat: {
      agg::gradient_repeat_adaptor<linear_type> grad_rep(linear);
      typedef agg::span_gradient<color, interpolator_type, agg::gradient_repeat_adaptor<linear_type>, color_func_type> span_repeat_type;
      
      span_repeat_type span_repeat(span_interpolator, 
                                   grad_rep, 
                                   gradient, 
                                   0, d2);
      
      typedef agg::renderer_scanline_aa<Render, span_allocator_type, span_repeat_type> renderer_repeat_type;
      renderer_repeat_type repeat_renderer(renderer, sa, span_repeat);
      
      render<agg::scanline_p8>(ras, ras_clip, sl, repeat_renderer, clip);
      break;
    }
    case ExtendPad: {
      typedef agg::span_gradient<color, interpolator_type, linear_type, color_func_type> span_pad_type;
      
      span_pad_type span_pad(span_interpolator, 
                             linear, 
                             gradient, 
                             0, d2);
      
      typedef agg::renderer_scanline_aa<Render, span_allocator_type, span_pad_type> renderer_pad_type;
      renderer_pad_type pad_renderer(renderer, sa, span_pad);
      
      render<agg::scanline_p8>(ras, ras_clip, sl, pad_renderer, clip);
      break;
    }
    case ExtendNone: {
      typedef agg::span_gradient<color, interpolator_type, linear_type, color_func_type> span_pad_type;
      
      span_pad_type span_pad(span_interpolator, 
                             linear, 
                             gradient, 
                             0, d2, false);
      
      typedef agg::renderer_scanline_aa<Render, span_allocator_type, span_pad_type> renderer_pad_type;
      renderer_pad_type pad_renderer(renderer, sa, span_pad);
      
      render<agg::scanline_p8>(ras, ras_clip, sl, pad_renderer, clip);
      break;
    }
    }
  }
  
  template<class Raster, class RasterClip, class Scanline, class Render>
  void draw_radial(Raster &ras, RasterClip &ras_clip, Scanline &sl, Render &renderer, bool clip) {
    interpolator_type span_interpolator(mtx);
    switch (extend) {
    case ExtendReflect: {
      agg::gradient_reflect_adaptor<radial_type> grad_refl(radial);
      typedef agg::span_gradient<color, interpolator_type, agg::gradient_reflect_adaptor<radial_type>, color_func_type> span_reflect_type;
      
      span_reflect_type span_reflect(span_interpolator, 
                                     grad_refl, 
                                     gradient, 
                                     0, d2);
      
      typedef agg::renderer_scanline_aa<Render, span_allocator_type, span_reflect_type> renderer_reflect_type;
      renderer_reflect_type reflect_renderer(renderer, sa, span_reflect);
      
      render<agg::scanline_p8>(ras, ras_clip, sl, reflect_renderer, clip);
      break;
    }
    case ExtendRepeat: {
      agg::gradient_repeat_adaptor<radial_type> grad_rep(radial);
      typedef agg::span_gradient<color, interpolator_type, agg::gradient_repeat_adaptor<radial_type>, color_func_type> span_repeat_type;
      
      span_repeat_type span_repeat(span_interpolator, 
                                   grad_rep, 
                                   gradient, 
                                   0, d2);
      
      typedef agg::renderer_scanline_aa<Render, span_allocator_type, span_repeat_type> renderer_repeat_type;
      renderer_repeat_type repeat_renderer(renderer, sa, span_repeat);
      
      render<agg::scanline_p8>(ras, ras_clip, sl, repeat_renderer, clip);
      break;
    }
    case ExtendPad: {
      typedef agg::span_gradient<color, interpolator_type, radial_type, color_func_type> span_pad_type;
      
      span_pad_type span_pad(span_interpolator, 
                             radial, 
                             gradient, 
                             0, d2);
      
      typedef agg::renderer_scanline_aa<Render, span_allocator_type, span_pad_type> renderer_pad_type;
      renderer_pad_type pad_renderer(renderer, sa, span_pad);
      
      render<agg::scanline_p8>(ras, ras_clip, sl, pad_renderer, clip);
      break;
    }
    case ExtendNone: {
      typedef agg::span_gradient<color, interpolator_type, radial_type, color_func_type> span_pad_type;
      
      span_pad_type span_pad(span_interpolator, 
                             radial, 
                             gradient, 
                             0, d2, false);
      
      typedef agg::renderer_scanline_aa<Render, span_allocator_type, span_pad_type> renderer_pad_type;
      renderer_pad_type pad_renderer(renderer, sa, span_pad);
      
      render<agg::scanline_p8>(ras, ras_clip, sl, pad_renderer, clip);
      break;
    }
    }
  }
  
  template<class Raster, class RasterClip, class Scanline, class Render>
  void draw_tile(Raster &ras, RasterClip &ras_clip, Scanline &sl, Render &renderer, bool clip) {
    interpolator_type span_interpolator(mtx);
    PIXFMT img_pixf(buffer.get_buffer());
    agg::span_allocator<color> sa;
    
    switch (extend) {
    case ExtendReflect: {
      typedef agg::image_accessor_wrap<PIXFMT, agg::wrap_mode_reflect, agg::wrap_mode_reflect> img_source_type;
      img_source_type img_src(img_pixf);
      typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> span_reflect_type;
      span_reflect_type span_reflect(img_src, span_interpolator);
      agg::renderer_scanline_aa<Render, span_allocator_type, span_reflect_type> reflect_renderer(renderer, sa, span_reflect);
      render<agg::scanline_p8>(ras, ras_clip, sl, reflect_renderer, clip);
      break;
    }
    case ExtendRepeat: {
      typedef agg::image_accessor_wrap<PIXFMT, agg::wrap_mode_repeat, agg::wrap_mode_repeat> img_source_type;
      img_source_type img_src(img_pixf);
      typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> span_repeat_type;
      span_repeat_type span_repeat(img_src, span_interpolator);
      agg::renderer_scanline_aa<Render, span_allocator_type, span_repeat_type> repeat_renderer(renderer, sa, span_repeat);
      render<agg::scanline_p8>(ras, ras_clip, sl, repeat_renderer, clip);
      break;
    }
    case ExtendPad: {
      typedef agg::image_accessor_clone<PIXFMT> img_source_type;
      img_source_type img_src(img_pixf);
      typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> span_pad_type;
      span_pad_type span_pad(img_src, span_interpolator);
      agg::renderer_scanline_aa<Render, span_allocator_type, span_pad_type> pad_renderer(renderer, sa, span_pad);
      render<agg::scanline_p8>(ras, ras_clip, sl, pad_renderer, clip);
      break;
    }
    case ExtendNone: {
      typedef agg::image_accessor_clip<PIXFMT> img_source_type;
      img_source_type img_src(img_pixf, color(0, 0, 0, 0));
      typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> span_none_type;
      span_none_type span_none(img_src, span_interpolator);
      agg::renderer_scanline_aa<Render, span_allocator_type, span_none_type> none_renderer(renderer, sa, span_none);
      render<agg::scanline_p8>(ras, ras_clip, sl, none_renderer, clip);
      break;
    }
    }
  }
};
