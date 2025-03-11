#ifndef R_NO_REMAP
#define R_NO_REMAP
#endif

#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>
#include <systemfonts.h>
#include "ragg.h"

static const R_CallMethodDef CallEntries[] = {
  {"agg_ppm_c", (DL_FUNC) &agg_ppm_c, 8},
  {"agg_png_c", (DL_FUNC) &agg_png_c, 9},
  {"agg_supertransparent_c", (DL_FUNC) &agg_supertransparent_c, 9},
  {"agg_tiff_c", (DL_FUNC) &agg_tiff_c, 11},
  {"agg_jpeg_c", (DL_FUNC) &agg_jpeg_c, 11},
  {"agg_capture_c", (DL_FUNC) &agg_capture_c, 8},
  {"agg_record_c", (DL_FUNC) &agg_record_c, 8},
  {NULL, NULL, 0}
};

extern "C" void R_init_ragg(DllInfo *dll) {
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}
