#include "ragg.h"
#include "init_device.h"

#include "AggDeviceJpeg.h"

// [[export]]
SEXP agg_jpeg_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg, 
                SEXP res, SEXP scaling, SEXP snap, SEXP quality, SEXP smoothing, 
                SEXP method) {
  int bgCol = RGBpar(bg, 0);
  
  BEGIN_CPP
    AggDeviceJpegNoAlpha* device = new AggDeviceJpegNoAlpha(
      Rf_translateCharUTF8((STRING_ELT(file, 0))), 
      INTEGER(width)[0], 
      INTEGER(height)[0], 
      REAL(pointsize)[0], 
      bgCol,
      REAL(res)[0],
      REAL(scaling)[0],
      LOGICAL(snap)[0],
      INTEGER(quality)[0],
      INTEGER(smoothing)[0],
      INTEGER(method)[0]
    );
  makeDevice<AggDeviceJpegNoAlpha>(device, "agg_jpeg");
  END_CPP
    
  return R_NilValue;
}
