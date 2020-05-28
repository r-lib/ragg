#ifndef RAGG_INCLUDED
#define RAGG_INCLUDED

#include <string>
#include <tuple>
#include <utility>
#include <unordered_map>

#define R_NO_REMAP

#include <Rinternals.h>
#include <R_ext/GraphicsEngine.h>

#include "agg_basics.h"

#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"

#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"

// Define a C++ try-catch macro to guard C++ calls
#define BEGIN_CPP try {

#define END_CPP                                                                \
}                                                                              \
catch (std::exception & e) {                                                   \
  Rf_error("C++ exception: %s", e.what());                                     \
}                                                                              \

#define R_USE_PROTOTYPES 1
typedef agg::pixfmt_rgb24_pre                   pixfmt_type_24;
typedef agg::pixfmt_rgba32_pre                  pixfmt_type_32;
typedef agg::pixfmt_rgb48_pre                   pixfmt_type_48;
typedef agg::pixfmt_rgba64_pre                  pixfmt_type_64;

#ifdef __BIG_ENDIAN__ 
typedef agg::pixfmt_abgr32_plain                pixfmt_r_raster;
#else
typedef agg::pixfmt_rgba32_plain                pixfmt_r_raster;
#endif

// pixfmt agnosting demultiplying
template<typename PIXFMT>
inline void demultiply(PIXFMT* pixfmt) {
  return;
}
template<>
inline void demultiply<pixfmt_type_32>(pixfmt_type_32* pixfmt) {
  pixfmt->demultiply();
  return;
}
template<>
inline void demultiply<pixfmt_type_64>(pixfmt_type_64* pixfmt) {
  pixfmt->demultiply();
  return;
}

SEXP agg_ppm_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
               SEXP res);
SEXP agg_png_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
               SEXP res, SEXP bit);
SEXP agg_supertransparent_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, 
                            SEXP bg, SEXP res, SEXP alpha_mod);
SEXP agg_tiff_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
                SEXP res, SEXP bit, SEXP compression, SEXP encoding);
SEXP agg_jpeg_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
                SEXP res, SEXP quality, SEXP smoothing, SEXP method);
SEXP agg_capture_c(SEXP name, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
                   SEXP res);

#endif
