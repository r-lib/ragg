#include <R.h>
#include <Rinternals.h>
#include <R_ext/GraphicsEngine.h>

#include "agg_basics.h"

#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"

#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"

#ifndef RAGG_INCLUDED
#define RAFF_INCLUDED

#define R_USE_PROTOTYPES 1

typedef agg::pixfmt_rgb24                   pixfmt_type;
typedef agg::pixfmt_rgb24_pre               pixfmt_pre_type;
typedef agg::pixfmt_rgba32                  pixfmt_r_raster;
typedef agg::renderer_base<pixfmt_type>     renbase_type;
typedef agg::renderer_base<pixfmt_pre_type> renbase_pre_type;
typedef agg::renderer_scanline_aa_solid<renbase_type> renderer_solid;
typedef agg::renderer_scanline_bin_solid<renbase_type> renderer_bin;

SEXP agg_ppm_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg);

#endif
