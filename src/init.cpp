#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

#include "ragg.h"

static font_map* fonts;

font_map& get_font_map(){
  return *fonts; 
}

const char * ragg_fcp, * ragg_fcf, * old_fcp, * old_fcf;
bool has_ragg_conf = false;

void set_ragg_fc() {
  static bool ragg_is_set = false;
  if (!ragg_is_set) {
    ragg_fcp = getenv("RAGG_FONTCONFIG_PATH");
    ragg_fcf = getenv("RAGG_FONTCONFIG_FILE");
    has_ragg_conf = ragg_fcp && ragg_fcf;
    old_fcp = getenv("FONTCONFIG_PATH");
    old_fcf = getenv("FONTCONFIG_FILE");
    ragg_is_set = true;
  }
  if (has_ragg_conf) {
    setenv("FONTCONFIG_PATH", ragg_fcp, 1);
    setenv("FONTCONFIG_FILE", ragg_fcf, 1);
  }
}
void unset_ragg_fc() {
  if (!has_ragg_conf) return;
  
  if (old_fcp) {
    setenv("FONTCONFIG_PATH", old_fcp, 1);
  } else {
    unsetenv("FONTCONFIG_PATH");
  }
  if (old_fcf) {
    setenv("FONTCONFIG_FILE", old_fcf, 1);
  } else {
    unsetenv("FONTCONFIG_FILE");
  }
}

static const R_CallMethodDef CallEntries[] = {
  {"agg_ppm_c", (DL_FUNC) &agg_ppm_c, 6},
  {"agg_png_c", (DL_FUNC) &agg_png_c, 7},
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
