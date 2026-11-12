#pragma once

#include <string>
#include <tuple>
#include <utility>
#include <unordered_map>
#include <stdexcept>
#include <cstdint>
#include <cstddef>

#ifndef R_NO_REMAP
#define R_NO_REMAP
#endif

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
catch (const std::bad_alloc&) {                                                \
  Rf_error("Memory allocation error. You are likely trying to create too large an image"); \
}                                                                              \
catch (std::exception & e) {                                                   \
  Rf_error("C++ exception: %s", e.what());                                     \
}                                                                              \

// Compute a pixel buffer size with overflow and sanity checks. Throws
// std::runtime_error (caught by END_CPP) on invalid or too-large sizes.
inline size_t checked_buffer_size(int64_t w, int64_t h, int64_t bpp) {
  if (w < 0 || h < 0 || bpp <= 0) {
    throw std::runtime_error("Invalid device dimensions");
  }
  int64_t size = w * h * bpp;
  if (static_cast<uint64_t>(size) > static_cast<uint64_t>(SIZE_MAX)) {
    throw std::runtime_error("Device dimensions are too large");
  }
  return static_cast<size_t>(size);
}

// Evaluate an R call while guaranteeing that a cleanup function runs
// afterwards, whether the call returns normally or longjmps (R error or
// interrupt). Device code uses this to restore rendering state that was
// changed before handing control back to R. R_UnwindProtect continues the
// jump by throwing a non-std::exception C++ exception, so destructors run
// and the END_CPP handlers below do not intercept it.
typedef void (*RaggCleanupFun)(void* data);

struct RaggUnwindData {
  SEXP expr;
  void* cleanup_data;
  RaggCleanupFun cleanup;
};

inline SEXP ragg_unwind_eval(void* data) {
  RaggUnwindData* d = static_cast<RaggUnwindData*>(data);
  SEXP R_fcall = PROTECT(Rf_lang1(d->expr));
  SEXP res = Rf_eval(R_fcall, R_GlobalEnv);
  UNPROTECT(1);
  return res;
}
inline void ragg_unwind_cleanup(void* data, Rboolean) {
  RaggUnwindData* d = static_cast<RaggUnwindData*>(data);
  d->cleanup(d->cleanup_data);
}
inline void ragg_eval_with_cleanup(SEXP expr, RaggCleanupFun cleanup, void* cleanup_data) {
  RaggUnwindData data = {expr, cleanup_data, cleanup};
  R_UnwindProtect(ragg_unwind_eval, &data, ragg_unwind_cleanup, &data, NULL);
}

#define R_USE_PROTOTYPES 1
typedef agg::pixfmt_rgb24_pre                   pixfmt_type_24;
typedef agg::pixfmt_rgba32_pre                  pixfmt_type_32;
typedef agg::pixfmt_rgb48_pre                   pixfmt_type_48;
typedef agg::pixfmt_rgba64_pre                  pixfmt_type_64;
typedef agg::pixfmt_bgra32_pre                  pixfmt_col_glyph;

#ifdef WORDS_BIGENDIAN
typedef agg::pixfmt_abgr32_plain                pixfmt_r_raster;
typedef agg::pixfmt_abgr32_pre                  pixfmt_r_capture;
#else
typedef agg::pixfmt_rgba32_plain                pixfmt_r_raster;
typedef agg::pixfmt_rgba32_pre                  pixfmt_r_capture;
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
               SEXP res, SEXP scaling, SEXP snap);
SEXP agg_png_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg,
               SEXP res, SEXP scaling, SEXP snap, SEXP bit);
SEXP agg_webp_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg,
                SEXP res, SEXP scaling, SEXP snap, SEXP lossy, SEXP quality);
SEXP agg_webp_anim_c(SEXP file, SEXP width, SEXP height, SEXP pointsize,
                     SEXP bg, SEXP res, SEXP scaling, SEXP snap, SEXP delay,
                     SEXP loop, SEXP lossy, SEXP quality);
SEXP agg_supertransparent_c(SEXP file, SEXP width, SEXP height, SEXP pointsize,
                            SEXP bg, SEXP res, SEXP scaling, SEXP snap,
                            SEXP alpha_mod);
SEXP agg_tiff_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg,
                SEXP res, SEXP scaling, SEXP snap, SEXP bit, SEXP compression,
                SEXP encoding);
SEXP agg_jpeg_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg,
                SEXP res, SEXP scaling, SEXP snap, SEXP quality, SEXP smoothing,
                SEXP method);
SEXP agg_capture_c(SEXP name, SEXP width, SEXP height, SEXP pointsize, SEXP bg,
                   SEXP res, SEXP scaling, SEXP snap);
SEXP agg_record_c(SEXP name, SEXP width, SEXP height, SEXP pointsize, SEXP bg,
                  SEXP res, SEXP scaling, SEXP snap);
