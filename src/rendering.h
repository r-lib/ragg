#pragma once

#include "ragg.h"
#include "agg_pixfmt_gray.h"

#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_scanline_boolean_algebra.h"
#include "util/agg_color_conv.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_allocator.h"

template<class ScanlineRes, class Raster, class RasterClip, class Scanline, class Render>
void render(Raster &ras, RasterClip &ras_clip, Scanline &sl, Render &renderer, bool clip) {
  if (clip) {
    ScanlineRes sl_result;
    agg::scanline_p8 sl_clip;
    agg::sbool_intersect_shapes_aa(ras, ras_clip, sl, sl_clip, sl_result, renderer);
  } else {
    agg::render_scanlines(ras, sl, renderer);
  }
}

template<class Source, class Target, class Raster, class RasterClip, class Scanline, class Render, class Interpolator>
void render_raster(agg::rendering_buffer &rbuf, unsigned w, unsigned h, Raster &ras, 
                   RasterClip &ras_clip, Scanline &sl, Interpolator interpolator, 
                   Render &renderer, bool interpolate, bool clip, bool scale_down) {
  unsigned char * buffer8 = new unsigned char[w * h * Target::pix_width];
  agg::rendering_buffer rbuf8(buffer8, w, h, w * Target::pix_width);
  agg::convert<Target, Source>(&rbuf8, &rbuf);
  
  typedef agg::image_accessor_clone<Target> img_source_type;
  Target img_pixf(rbuf8);
  img_source_type img_src(img_pixf);
  agg::span_allocator<typename Render::color_type> sa;
  
  if (interpolate) {
    typedef agg::span_image_filter_rgba_bilinear<img_source_type, Interpolator> span_gen_type;
    span_gen_type sg(img_src, interpolator);
    agg::renderer_scanline_aa<Render, agg::span_allocator<typename Render::color_type>, span_gen_type> raster_renderer(renderer, sa, sg);
    render<agg::scanline_p8>(ras, ras_clip, sl, raster_renderer, clip);
  } else if (scale_down) {
    typedef agg::span_image_resample_rgba_affine<img_source_type> span_gen_type;
    agg::image_filter_bilinear filter_kernel;
    agg::image_filter_lut filter(filter_kernel, true);
    span_gen_type sg(img_src, interpolator, filter);
    sg.blur(1);
    agg::renderer_scanline_aa<Render, agg::span_allocator<typename Render::color_type>, span_gen_type> raster_renderer(renderer, sa, sg);
    render<agg::scanline_u8>(ras, ras_clip, sl, raster_renderer, clip);
  } else {
    typedef agg::span_image_filter_rgba_nn<img_source_type, Interpolator> span_gen_type;
    span_gen_type sg(img_src, interpolator);
    agg::renderer_scanline_aa<Render, agg::span_allocator<typename Render::color_type>, span_gen_type> raster_renderer(renderer, sa, sg);
    render<agg::scanline_p8>(ras, ras_clip, sl, raster_renderer, clip);
  }
  
  delete [] buffer8;
}
