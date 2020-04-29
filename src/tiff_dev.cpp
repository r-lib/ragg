#include "ragg.h"
#include "init_device.h"

#include "AggDeviceTiff.h"

// [[export]]
SEXP agg_tiff_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
                SEXP res, SEXP bit, SEXP compression, SEXP encoding) {
  bool bit8 = INTEGER(bit)[0] == 8;
  int bgCol = RGBpar(bg, 0);
  
  BEGIN_CPP
  if (bit8) {
    if (R_ALPHA(bgCol) == 255) { // Opaque bg... no need for alpha channel
      AggDeviceTiffNoAlpha* device = new AggDeviceTiffNoAlpha(
        CHAR(STRING_ELT(file, 0)), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0],
        INTEGER(compression)[0],
        INTEGER(encoding)[0]
      );
      makeDevice<AggDeviceTiffNoAlpha>(device, "agg_tiff");
    } else {
      AggDeviceTiffAlpha* device = new AggDeviceTiffAlpha(
        CHAR(STRING_ELT(file, 0)), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0],
        INTEGER(compression)[0],
        INTEGER(encoding)[0]
      );
      makeDevice<AggDeviceTiffAlpha>(device, "agg_tiff");
    }
  } else {
    if (R_ALPHA(bgCol) == 255) { // Opaque bg... no need for alpha channel
      AggDeviceTiff16NoAlpha* device = new AggDeviceTiff16NoAlpha(
        CHAR(STRING_ELT(file, 0)), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0],
        INTEGER(compression)[0],
        INTEGER(encoding)[0]
      );
      makeDevice<AggDeviceTiff16NoAlpha>(device, "agg_png");
    } else {
      AggDeviceTiff16Alpha* device = new AggDeviceTiff16Alpha(
        CHAR(STRING_ELT(file, 0)), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0],
        INTEGER(compression)[0],
        INTEGER(encoding)[0]
      );
      makeDevice<AggDeviceTiff16Alpha>(device, "agg_png");
    }
  }
  END_CPP
  
  return R_NilValue;
}
