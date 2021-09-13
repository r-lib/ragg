#pragma once

#include "ragg.h"

#include "agg_scanline_p.h"
#include "agg_scanline_boolean_algebra.h"

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
