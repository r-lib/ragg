#include "ragg.h"
#include "init_device.h"

#include "AggDeviceSixel.h"

// [[export]]
SEXP agg_sixel_c(SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
               SEXP res, SEXP scaling, SEXP snap) {
  int bgCol = RGBpar(bg, 0);
  if (R_TRANSPARENT(bgCol)) {
    bgCol = R_TRANWHITE;
  }
  
  BEGIN_CPP
  AggDeviceSixelNoAlpha* device = new AggDeviceSixelNoAlpha(
    INTEGER(width)[0], 
    INTEGER(height)[0], 
    REAL(pointsize)[0], 
    bgCol,
    REAL(res)[0],
    REAL(scaling)[0],
    LOGICAL(snap)[0]
  );
  makeDevice<AggDeviceSixelNoAlpha>(device, "agg_sixel");
  END_CPP
  
  return R_NilValue;
}
