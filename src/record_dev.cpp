#include "ragg.h"
#include "init_device.h"

#include "AggDeviceRecord.h"

template<>
void agg_clip<AggDeviceRecordAlpha>(double x0, double x1, double y0, double y1, pDevDesc dd) {
  return;
}

template<>
void agg_new_page<AggDeviceRecordAlpha>(const pGEcontext gc, pDevDesc dd) {
  return;
}

template<>
void agg_close<AggDeviceRecordAlpha>(pDevDesc dd) {
  return;
}

template<>
void agg_line<AggDeviceRecordAlpha>(double x1, double y1, double x2, double y2,
              const pGEcontext gc, pDevDesc dd) {
  return;
}

template<>
void agg_polyline<AggDeviceRecordAlpha>(int n, double *x, double *y, const pGEcontext gc,
                  pDevDesc dd) {
  return;
}

template<>
void agg_polygon<AggDeviceRecordAlpha>(int n, double *x, double *y, const pGEcontext gc,
                 pDevDesc dd) {
  return;
}

template<>
void agg_path<AggDeviceRecordAlpha>(double *x, double *y, int npoly, int *nper, Rboolean winding,
              const pGEcontext gc, pDevDesc dd) {
  return;
}

template<>
void agg_rect<AggDeviceRecordAlpha>(double x0, double y0, double x1, double y1, const pGEcontext gc,
              pDevDesc dd) {
  return;
}

template<>
void agg_circle<AggDeviceRecordAlpha>(double x, double y, double r, const pGEcontext gc,
                pDevDesc dd) {
  return;
}

template<>
void agg_text<AggDeviceRecordAlpha>(double x, double y, const char *str, double rot, double hadj,
              const pGEcontext gc, pDevDesc dd) {
  return;
}

template<>
void agg_raster<AggDeviceRecordAlpha>(unsigned int *raster, int w, int h, double x, double y,
                double width, double height, double rot, Rboolean interpolate,
                const pGEcontext gc, pDevDesc dd) {
  return;
}

template<>
void agg_useGroup<AggDeviceRecordAlpha>(SEXP ref, SEXP trans, pDevDesc dd) {
  return;
}

template<>
void agg_stroke<AggDeviceRecordAlpha>(SEXP path, const pGEcontext gc, pDevDesc dd) {
  return;
}

template<>
void agg_fill<AggDeviceRecordAlpha>(SEXP path, int rule, const pGEcontext gc, pDevDesc dd) {
  return;
}

template<>
void agg_fillStroke<AggDeviceRecordAlpha>(SEXP path, int rule, const pGEcontext gc, pDevDesc dd) {
  return;
}

template<>
void agg_glyph<AggDeviceRecordAlpha>(int n, int *glyphs, double *x, double *y,
               SEXP font, double size,
               int colour, double rot,
               pDevDesc dd) {
  return;
}

// [[export]]
SEXP agg_record_c(SEXP name, SEXP width, SEXP height, SEXP pointsize, SEXP bg, SEXP res, SEXP scaling, SEXP snap) {
  int bgCol = RGBpar(bg, 0);

  BEGIN_CPP
  AggDeviceRecordAlpha* device = new AggDeviceRecordAlpha(
    INTEGER(width)[0],
    INTEGER(height)[0],
    REAL(pointsize)[0],
    bgCol,
    REAL(res)[0],
    REAL(scaling)[0],
    LOGICAL(snap)[0]
  );
  makeDevice<AggDeviceRecordAlpha>(device, CHAR(STRING_ELT(name, 0)), true);
  END_CPP

  return R_NilValue;
}
