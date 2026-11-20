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

// In-memory access to the encoder for unit tests. Takes a raw RGB buffer
// (3 bytes per pixel, row-major) and returns the sixel string without
// writing anything to stdout.
SEXP sixel_encode_c(SEXP data, SEXP width, SEXP height) {
  BEGIN_CPP
  std::string sixel = sixeldev_impl::as_sixel(
    RAW(data),
    (size_t) INTEGER(width)[0],
    (size_t) INTEGER(height)[0]
  );
  SEXP res = PROTECT(Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(res, 0, Rf_mkCharLenCE(sixel.data(), sixel.size(), CE_NATIVE));
  UNPROTECT(1);
  return res;
  END_CPP
}
