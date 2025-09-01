#include "ragg.h"
#include "init_device.h"
#include "AggDeviceWebP.h"

// [[export]]
SEXP agg_webp_c(SEXP file, SEXP width, SEXP height, SEXP pointsize, SEXP bg,
               SEXP res, SEXP scaling, SEXP snap, SEXP lossy, SEXP quality) {
  int bgCol = RGBpar(bg, 0);
  bool los = LOGICAL(lossy)[0];
  int qual = INTEGER(quality)[0];

  BEGIN_CPP
  if (R_OPAQUE(bgCol)) { // Opaque bg... no need for alpha channel
    AggDeviceWebPNoAlpha* device = new AggDeviceWebPNoAlpha(
      Rf_translateCharUTF8((STRING_ELT(file, 0))),
      INTEGER(width)[0],
      INTEGER(height)[0],
      REAL(pointsize)[0],
      bgCol,
      REAL(res)[0],
      REAL(scaling)[0],
      LOGICAL(snap)[0],
      los,
      qual
    );
    makeDevice<AggDeviceWebPNoAlpha>(device, "agg_webp");
  } else {
    AggDeviceWebPAlpha* device = new AggDeviceWebPAlpha(
      Rf_translateCharUTF8((STRING_ELT(file, 0))),
      INTEGER(width)[0],
      INTEGER(height)[0],
      REAL(pointsize)[0],
      bgCol,
      REAL(res)[0],
      REAL(scaling)[0],
      LOGICAL(snap)[0],
      los,
      qual
    );
    makeDevice<AggDeviceWebPAlpha>(device, "agg_webp");
  }
  END_CPP

  return R_NilValue;
}
