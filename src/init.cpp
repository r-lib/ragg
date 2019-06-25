#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>
#include "ragg.h"

static font_map* fonts;

font_map& get_font_map(){
  return *fonts; 
}

static const R_CallMethodDef CallEntries[] = {
  {"agg_ppm_c", (DL_FUNC) &agg_ppm_c, 6},
  {"agg_png_c", (DL_FUNC) &agg_png_c, 7},
  {"agg_supertransparent_c", (DL_FUNC) &agg_supertransparent_c, 7},
  {"agg_tiff_c", (DL_FUNC) &agg_tiff_c, 9},
  {"agg_capture_c", (DL_FUNC) &agg_capture_c, 6},
  {NULL, NULL, 0}
};

extern "C" void R_init_ragg(DllInfo *dll) {
  fonts = new font_map();
  
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}

extern "C" void R_unload_ragg(DllInfo *dll) {
  delete fonts;
}
