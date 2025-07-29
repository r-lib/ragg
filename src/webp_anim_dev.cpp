#include "ragg.h"
#include "init_device.h"
#include "AggDeviceWebPAnim.h"

// [[export]]
SEXP agg_webp_anim_c(SEXP file, SEXP width, SEXP height, SEXP pointsize,
                     SEXP bg, SEXP res, SEXP scaling, SEXP snap_rect,
                     SEXP lossy, SEXP quality, SEXP delay, SEXP loop) {
  const char* filename = Rf_translateCharUTF8(STRING_ELT(file, 0));
  int w = INTEGER(width)[0];
  int h = INTEGER(height)[0];
  double ps = REAL(pointsize)[0];
  int bgCol = RGBpar(bg, 0);
  double dpi = REAL(res)[0];
  double scale = REAL(scaling)[0];
  bool snap = LOGICAL(snap_rect)[0];
  bool lossy_ = LOGICAL(lossy)[0];
  int quality_ = INTEGER(quality)[0];
  int delay_ms = INTEGER(delay)[0];
  int loop_count = INTEGER(loop)[0];

  BEGIN_CPP
  if (R_OPAQUE(bgCol)) {
    auto device = new AggDeviceWebPAnimNoAlpha(
        filename, w, h, ps, bgCol, dpi, scale, snap, lossy_, quality_,
        delay_ms, loop_count);
    makeDevice<AggDeviceWebPAnimNoAlpha>(device, "agg_webp_anim");
  } else {
    auto device = new AggDeviceWebPAnimAlpha(
        filename, w, h, ps, bgCol, dpi, scale, snap, lossy_, quality_,
        delay_ms, loop_count);
    makeDevice<AggDeviceWebPAnimAlpha>(device, "agg_webp_anim");
  }
  END_CPP

  return R_NilValue;
}
