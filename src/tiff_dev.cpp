#include "ragg.h"
#include "init_device.h"

#include "AggDeviceTiff.h"

// [[export]]
SEXP agg_tiff_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
                SEXP res, SEXP scaling, SEXP snap, SEXP bit, SEXP compression, 
                SEXP encoding) {
  bool bit8 = INTEGER(bit)[0] == 8;
  int bgCol = RGBpar(bg, 0);
  
  BEGIN_CPP
  if (bit8) {
    if (R_OPAQUE(bgCol)) { // Opaque bg... no need for alpha channel
      AggDeviceTiffNoAlpha* device = new AggDeviceTiffNoAlpha(
        Rf_translateCharUTF8((STRING_ELT(file, 0))), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0],
        REAL(scaling)[0],
        LOGICAL(snap)[0],
        INTEGER(compression)[0],
        INTEGER(encoding)[0]
      );
      makeDevice<AggDeviceTiffNoAlpha>(device, "agg_tiff");
    } else {
      AggDeviceTiffAlpha* device = new AggDeviceTiffAlpha(
        Rf_translateCharUTF8((STRING_ELT(file, 0))), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0],
        REAL(scaling)[0],
        LOGICAL(snap)[0],
        INTEGER(compression)[0],
        INTEGER(encoding)[0]
      );
      makeDevice<AggDeviceTiffAlpha>(device, "agg_tiff");
    }
  } else {
    if (R_OPAQUE(bgCol)) { // Opaque bg... no need for alpha channel
      AggDeviceTiff16NoAlpha* device = new AggDeviceTiff16NoAlpha(
        Rf_translateCharUTF8((STRING_ELT(file, 0))), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0],
        REAL(scaling)[0],
        LOGICAL(snap)[0],
        INTEGER(compression)[0],
        INTEGER(encoding)[0]
      );
      makeDevice<AggDeviceTiff16NoAlpha>(device, "agg_tiff");
    } else {
      AggDeviceTiff16Alpha* device = new AggDeviceTiff16Alpha(
        Rf_translateCharUTF8((STRING_ELT(file, 0))), 
        INTEGER(width)[0], 
        INTEGER(height)[0], 
        REAL(pointsize)[0], 
        bgCol,
        REAL(res)[0],
        REAL(scaling)[0],
        LOGICAL(snap)[0],
        INTEGER(compression)[0],
        INTEGER(encoding)[0]
      );
      makeDevice<AggDeviceTiff16Alpha>(device, "agg_tiff");
    }
  }
  END_CPP
  
  return R_NilValue;
}
