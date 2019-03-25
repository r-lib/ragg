#include "ragg.h"
#include "init_device.h"

#include "AggDevice.h"

// [[export]]
SEXP agg_ppm_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg) {
  int bgCol = RGBpar(bg, 0);
  AggDevice* device = new AggDevice(
    CHAR(STRING_ELT(file, 0)), 
    INTEGER(width)[0], 
    INTEGER(height)[0], 
    REAL(pointsize)[0], 
    bgCol);
  makeDevice<AggDevice>(device, "agg_ppm");
  
  return R_NilValue;
}
