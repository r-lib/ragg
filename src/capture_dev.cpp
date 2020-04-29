#include "ragg.h"
#include "init_device.h"

#include "AggDeviceCapture.h"

// [[export]]
SEXP agg_capture_c(SEXP name, SEXP width, SEXP height, SEXP pointsize, SEXP bg, SEXP res) {
  int bgCol = RGBpar(bg, 0);
  
  BEGIN_CPP
  AggDeviceCaptureAlpha* device = new AggDeviceCaptureAlpha(
    "", 
    INTEGER(width)[0], 
    INTEGER(height)[0], 
    REAL(pointsize)[0], 
    bgCol,
    REAL(res)[0]
  );
  makeDevice<AggDeviceCaptureAlpha>(device, CHAR(STRING_ELT(name, 0)));
  END_CPP
  
  return R_NilValue;
}
