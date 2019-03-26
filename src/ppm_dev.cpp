#include "ragg.h"
#include "init_device.h"

#include "AggDevicePpm.h"

// [[export]]
SEXP agg_ppm_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
               SEXP res) {
  int bgCol = RGBpar(bg, 0);
  AggDevicePpmNoAlpha* device = new AggDevicePpmNoAlpha(
    CHAR(STRING_ELT(file, 0)), 
    INTEGER(width)[0], 
    INTEGER(height)[0], 
    REAL(pointsize)[0], 
    bgCol,
    REAL(res)[0]
  );
  makeDevice<AggDevicePpmNoAlpha>(device, "agg_ppm");
  
  return R_NilValue;
}
