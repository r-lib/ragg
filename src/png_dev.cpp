#include "ragg.h"
#include "init_device.h"

#include "AggDevicePng.h"

// [[export]]
SEXP agg_png_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, SEXP res, SEXP bit) {
  bool bit8 = INTEGER(bit)[0] == 8;
  int bgCol = RGBpar(bg, 0);
  if (bit8) {
    if (R_ALPHA(bgCol) == 255) { // Opaque bg... no need for alpha channel
      AggDevicePngNoAlpha* device = new AggDevicePngNoAlpha(
        CHAR(STRING_ELT(file, 0)), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0]
      );
      makeDevice<AggDevicePngNoAlpha>(device, "agg_png");
    } else {
      AggDevicePngAlpha* device = new AggDevicePngAlpha(
        CHAR(STRING_ELT(file, 0)), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0]
      );
      makeDevice<AggDevicePngAlpha>(device, "agg_png");
    }
  } else {
    if (R_ALPHA(bgCol) == 255) { // Opaque bg... no need for alpha channel
      AggDevicePng16NoAlpha* device = new AggDevicePng16NoAlpha(
        CHAR(STRING_ELT(file, 0)), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0]
      );
      makeDevice<AggDevicePng16NoAlpha>(device, "agg_png");
    } else {
      AggDevicePng16Alpha* device = new AggDevicePng16Alpha(
        CHAR(STRING_ELT(file, 0)), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0]
      );
      makeDevice<AggDevicePng16Alpha>(device, "agg_png");
    }
  }
  
  return R_NilValue;
}

SEXP agg_supertransparent_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, SEXP res, SEXP alpha_mod) {
  int bgCol = RGBpar(bg, 0);
  if (R_ALPHA(bgCol) == 255) { // Opaque bg... no need for alpha channel
    AggDevicePng16NoAlpha* device = new AggDevicePng16NoAlpha(
      CHAR(STRING_ELT(file, 0)), 
      INTEGER(width)[0], 
      INTEGER(height)[0], 
      REAL(pointsize)[0], 
      bgCol,
      REAL(res)[0],
      REAL(alpha_mod)[0]
    );
    makeDevice<AggDevicePng16NoAlpha>(device, "agg_png");
  } else {
    AggDevicePng16Alpha* device = new AggDevicePng16Alpha(
      CHAR(STRING_ELT(file, 0)), 
      INTEGER(width)[0], 
      INTEGER(height)[0], 
      REAL(pointsize)[0], 
      bgCol,
      REAL(res)[0],
      REAL(alpha_mod)[0]
    );
    makeDevice<AggDevicePng16Alpha>(device, "agg_png");
  }
}
