#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

#include "ragg.h"

static const R_CallMethodDef CallEntries[] = {
  {"agg_ppm_c", (DL_FUNC) &agg_ppm_c, 5},
  {NULL, NULL, 0}
};

extern "C" void R_init_ragg(DllInfo *dll)
{
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}
