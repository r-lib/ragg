#ifndef RAGG_INCLUDED
#define RAGG_INCLUDED

#include <string>
#include <tuple>
#include <utility>
#include <unordered_map>

#include <R.h>
#include <Rinternals.h>
#include <R_ext/GraphicsEngine.h>

#include "agg_basics.h"

#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"

#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"

#define R_USE_PROTOTYPES 1
typedef agg::pixfmt_rgb24_pre                   pixfmt_type_24;
typedef agg::pixfmt_rgba32_pre                  pixfmt_type_32;
typedef agg::pixfmt_rgb48_pre                   pixfmt_type_48;
typedef agg::pixfmt_rgba64_pre                  pixfmt_type_64;

#ifdef __BIG_ENDIAN__ 
typedef agg::pixfmt_abgr32                      pixfmt_r_raster;
#else
typedef agg::pixfmt_rgba32                      pixfmt_r_raster;
#endif

#if defined(_WIN32)
#include <stdint.h>
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
#endif


typedef std::tuple<std::string, int, int> font_key;

struct key_hash : public std::unary_function<font_key, std::size_t>
{
  inline std::size_t operator()(const font_key& k) const
  {
    return std::get<0>(k)[0] ^ std::get<1>(k) ^ std::get<2>(k);
  }
};

struct key_equal : public std::binary_function<font_key, font_key, bool>
{
  inline bool operator()(const font_key& v0, const font_key& v1) const
  {
    return (
        std::get<0>(v0) == std::get<0>(v1) &&
          std::get<1>(v0) == std::get<1>(v1) &&
          std::get<2>(v0) == std::get<2>(v1)
    );
  }
};

typedef std::unordered_map<font_key, std::pair< std::string, int >, key_hash, key_equal> font_map;


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
SEXP agg_capture_c(SEXP name, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
                   SEXP res);

#endif
