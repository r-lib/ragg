#include "ragg.h"
#include "init_device.h"

#include "AggDevicePpm.h"

// [[export]]
SEXP agg_ppm_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
               SEXP res, SEXP scaling, SEXP snap) {
  int bgCol = RGBpar(bg, 0);
  if (R_TRANSPARENT(bgCol)) {
    bgCol = R_TRANWHITE;
  }
  
  BEGIN_CPP
  AggDevicePpmNoAlpha* device = new AggDevicePpmNoAlpha(
    Rf_translateCharUTF8((STRING_ELT(file, 0))), 
    INTEGER(width)[0], 
    INTEGER(height)[0], 
    REAL(pointsize)[0], 
    bgCol,
    REAL(res)[0],
    REAL(scaling)[0],
    LOGICAL(snap)[0]
  );
  makeDevice<AggDevicePpmNoAlpha>(device, "agg_ppm");
  END_CPP
  
  return R_NilValue;
}
