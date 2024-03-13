#pragma once

#include "ragg.h"

template<class T>
void agg_metric_info(int c, const pGEcontext gc, double* ascent,
                     double* descent, double* width, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  device->charMetric(c, gc->fontfamily, gc->fontface, gc->ps * gc->cex,
                     ascent, descent, width);
  END_CPP
    
  return;
}

template<class T>
void agg_clip(double x0, double x1, double y0, double y1, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  device->clipRect(x0, y0, x1, y1);
  END_CPP
  
  return;
}

template<class T>
void agg_new_page(const pGEcontext gc, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  device->newPage(gc->fill);
  END_CPP
  
  return;
}

template<class T>
void agg_close(pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  device->close();
  delete device;
  END_CPP
  
  return;
}

template<class T>
void agg_line(double x1, double y1, double x2, double y2,
              const pGEcontext gc, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  device->drawLine(x1, y1, x2, y2, gc->col, gc->lwd, gc->lty, gc->lend);
  END_CPP
  
  return;
}

template<class T>
void agg_polyline(int n, double *x, double *y, const pGEcontext gc,
                  pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  device->drawPolyline(n, x, y, gc->col, gc->lwd, gc->lty, gc->lend, gc->ljoin, 
                       gc->lmitre);
  END_CPP
  
  return;
}

template<class T>
void agg_polygon(int n, double *x, double *y, const pGEcontext gc,
                 pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  int pattern = -1;
#if R_GE_version >= 13
  pattern = gc->patternFill == R_NilValue ? -1 : INTEGER(gc->patternFill)[0];
#endif
  
  BEGIN_CPP
  device->drawPolygon(n, x, y, gc->fill, gc->col, gc->lwd, gc->lty, gc->lend, 
                      gc->ljoin, gc->lmitre, pattern);
  END_CPP
  
  return;
}

template<class T>
void agg_path(double *x, double *y, int npoly, int *nper, Rboolean winding,
              const pGEcontext gc, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  int pattern = -1;
#if R_GE_version >= 13
  pattern = gc->patternFill == R_NilValue ? -1 : INTEGER(gc->patternFill)[0];
#endif
  
  BEGIN_CPP
  device->drawPath(npoly, nper, x, y, gc->col, gc->fill, gc->lwd, gc->lty, 
                   gc->lend, gc->ljoin, gc->lmitre, !winding, pattern);
  END_CPP
  
  return;
}

template<class T>
double agg_strwidth(const char *str, const pGEcontext gc, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  return device->stringWidth(str, gc->fontfamily, gc->fontface, 
                             gc->ps * gc->cex);
  END_CPP
}

template<class T>
void agg_rect(double x0, double y0, double x1, double y1, const pGEcontext gc, 
              pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  int pattern = -1;
#if R_GE_version >= 13
  pattern = gc->patternFill == R_NilValue ? -1 : INTEGER(gc->patternFill)[0];
#endif
  
  BEGIN_CPP
  device->drawRect(x0, y0, x1, y1, gc->fill, gc->col, gc->lwd, 
                   gc->lty, gc->lend, pattern);
  END_CPP
  
  return;
}

template<class T>
void agg_circle(double x, double y, double r, const pGEcontext gc, 
                pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  int pattern = -1;
#if R_GE_version >= 13
  pattern = gc->patternFill == R_NilValue ? -1 : INTEGER(gc->patternFill)[0];
#endif
  
  BEGIN_CPP
  device->drawCircle(x, y, r, gc->fill, gc->col, gc->lwd, gc->lty, gc->lend, pattern);
  END_CPP
  
  return;
}

template<class T>
void agg_text(double x, double y, const char *str, double rot, double hadj, 
              const pGEcontext gc, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  device->drawText(x, y, str, gc->fontfamily, gc->fontface, gc->ps * gc->cex, 
                   rot, hadj, gc->col);
  END_CPP
  
  return;
}

template<class T>
static inline void agg_size(double *left, double *right, double *bottom, 
                            double *top, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  *left = dd->left;
  *right = (double) device->width;
  *bottom = (double) device->height;
  *top = dd->top;
}

template<class T>
void agg_raster(unsigned int *raster, int w, int h, double x, double y,
                double width, double height, double rot, Rboolean interpolate,
                const pGEcontext gc, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  device->drawRaster(raster, w, h, x, y, width, height, rot, interpolate);
  END_CPP
  
  return;
}

template<class T>
SEXP agg_capture(pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  return device->capture();
  END_CPP
}

template<class T>
SEXP agg_setPattern(SEXP pattern, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  return device->createPattern(pattern);
  END_CPP
    
  
  return R_NilValue;
}

template<class T>
void agg_releasePattern(SEXP ref, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  BEGIN_CPP
  device->removePattern(ref);
  END_CPP
} 

template<class T>
SEXP agg_setClipPath(SEXP path, SEXP ref, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  return device->createClipPath(path, ref);
  END_CPP
    
  return R_NilValue;
}

template<class T>
void agg_releaseClipPath(SEXP ref, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  BEGIN_CPP
  device->removeClipPath(ref);
  END_CPP
}

template<class T>
SEXP agg_setMask(SEXP path, SEXP ref, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  return device->createMask(path, ref);
  END_CPP
    
  return R_NilValue;
}

template<class T>
void agg_releaseMask(SEXP ref, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  BEGIN_CPP
  device->removeMask(ref);
  END_CPP
}

template<class T>
SEXP agg_defineGroup(SEXP source, int op, SEXP destination, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  BEGIN_CPP
  return device->renderGroup(source, op, destination);
  END_CPP
    
  return R_NilValue;
}

template<class T>
void agg_useGroup(SEXP ref, SEXP trans, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  BEGIN_CPP
  device->useGroup(ref, trans);
  END_CPP
}

template<class T>
void agg_releaseGroup(SEXP ref, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  BEGIN_CPP
  device->removeGroup(ref);
  END_CPP
}

template<class T>
void agg_stroke(SEXP path, const pGEcontext gc, pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  
  BEGIN_CPP
  device->renderPath(path, false, true, gc->col, gc->fill, gc->lwd, gc->lty, 
                     gc->lend, gc->ljoin, gc->lmitre, false, -1);
  END_CPP
    
  return;
}

template<class T>
void agg_fill(SEXP path, int rule, const pGEcontext gc, pDevDesc dd) {
  
#if R_GE_version >= 15
  T * device = (T *) dd->deviceSpecific;
  
  int pattern = gc->patternFill == R_NilValue ? -1 : INTEGER(gc->patternFill)[0];
  
  bool use_evenodd = rule == R_GE_evenOddRule;
  
  BEGIN_CPP
  device->renderPath(path, true, false, gc->col, gc->fill, gc->lwd, gc->lty, 
                     gc->lend, gc->ljoin, gc->lmitre, use_evenodd, pattern);
  END_CPP
#endif    
  
  return;
}

template<class T>
void agg_fillStroke(SEXP path, int rule, const pGEcontext gc, pDevDesc dd) {
  
#if R_GE_version >= 15
  T * device = (T *) dd->deviceSpecific;
  
  int pattern = gc->patternFill == R_NilValue ? -1 : INTEGER(gc->patternFill)[0];
  
  bool use_evenodd = rule == R_GE_evenOddRule;
  
  BEGIN_CPP
  device->renderPath(path, true, true, gc->col, gc->fill, gc->lwd, gc->lty, 
                     gc->lend, gc->ljoin, gc->lmitre, use_evenodd, pattern);
  END_CPP
#endif
    
  return;
}

template<class T>
void agg_glyph(int n, int *glyphs, double *x, double *y, 
               SEXP font, double size, 
               int colour, double rot,
               pDevDesc dd) {
  T * device = (T *) dd->deviceSpecific;
  BEGIN_CPP
  device->drawGlyph(n, glyphs, x, y, font, size, colour, rot);
  END_CPP
}

template<class T>
SEXP agg_capabilities(SEXP capabilities) {
#if R_GE_version >= 15
  // Pattern support
  SEXP pat = PROTECT(Rf_allocVector(INTSXP, 3));
  INTEGER(pat)[0] = R_GE_linearGradientPattern;
  INTEGER(pat)[1] = R_GE_radialGradientPattern;
  INTEGER(pat)[2] = R_GE_tilingPattern;
  SET_VECTOR_ELT(capabilities, R_GE_capability_patterns, pat);
  UNPROTECT(1);
  
  // Clipping path support
  SET_VECTOR_ELT(capabilities, R_GE_capability_clippingPaths, Rf_ScalarInteger(1));
  
  // Mask support
  SEXP masks = PROTECT(Rf_allocVector(INTSXP, 2));
  INTEGER(masks)[0] = R_GE_alphaMask;
  INTEGER(masks)[1] = R_GE_luminanceMask;
  SET_VECTOR_ELT(capabilities, R_GE_capability_masks, masks);
  UNPROTECT(1);
  
  // Group composition
  SEXP compositing = PROTECT(Rf_allocVector(INTSXP, 24));
  INTEGER(compositing)[13] = R_GE_compositeOver;
  INTEGER(compositing)[11] = R_GE_compositeClear;
  INTEGER(compositing)[0] = R_GE_compositeMultiply;
  INTEGER(compositing)[1] = R_GE_compositeScreen;
  INTEGER(compositing)[2] = R_GE_compositeOverlay;
  INTEGER(compositing)[3] = R_GE_compositeDarken;
  INTEGER(compositing)[4] = R_GE_compositeLighten;
  INTEGER(compositing)[5] = R_GE_compositeColorDodge;
  INTEGER(compositing)[6] = R_GE_compositeColorBurn;
  INTEGER(compositing)[7] = R_GE_compositeHardLight;
  INTEGER(compositing)[8] = R_GE_compositeSoftLight;
  INTEGER(compositing)[9] = R_GE_compositeDifference;
  INTEGER(compositing)[10] = R_GE_compositeExclusion;
  INTEGER(compositing)[12] = R_GE_compositeSource;
  INTEGER(compositing)[14] = R_GE_compositeIn;
  INTEGER(compositing)[15] = R_GE_compositeOut;
  INTEGER(compositing)[16] = R_GE_compositeAtop;
  INTEGER(compositing)[17] = R_GE_compositeDest;
  INTEGER(compositing)[18] = R_GE_compositeDestOver;
  INTEGER(compositing)[19] = R_GE_compositeDestIn;
  INTEGER(compositing)[20] = R_GE_compositeDestOut;
  INTEGER(compositing)[21] = R_GE_compositeDestAtop;
  INTEGER(compositing)[22] = R_GE_compositeXor;
  INTEGER(compositing)[23] = R_GE_compositeAdd;
  SET_VECTOR_ELT(capabilities, R_GE_capability_compositing, compositing);
  UNPROTECT(1);
  
  // Group transformation
  SET_VECTOR_ELT(capabilities, R_GE_capability_transformations, Rf_ScalarInteger(1));
  
  // Path stroking and filling
  SET_VECTOR_ELT(capabilities, R_GE_capability_paths, Rf_ScalarInteger(1));

#endif

  return capabilities;
}

static unsigned int DEVICE_COUNTER = 0;

template<class T>
pDevDesc agg_device_new(T* device) {
  
  pDevDesc dd = (DevDesc*) calloc(1, sizeof(DevDesc));
  if (dd == NULL)
    return dd;
  
  dd->startfill = device->background_int;
  dd->startcol = R_RGB(0, 0, 0);
  dd->startps = device->pointsize;
  dd->startlty = LTY_SOLID;
  dd->startfont = 1;
  dd->startgamma = 1;
  
  // Callbacks
  dd->activate = NULL;
  dd->deactivate = NULL;
  dd->close = agg_close<T>;
  dd->clip = agg_clip<T>;
  dd->size = agg_size<T>;
  dd->newPage = agg_new_page<T>;
  dd->line = agg_line<T>;
  dd->text = agg_text<T>;
  dd->strWidth = agg_strwidth<T>;
  dd->rect = agg_rect<T>;
  dd->circle = agg_circle<T>;
  dd->polygon = agg_polygon<T>;
  dd->polyline = agg_polyline<T>;
  dd->path = agg_path<T>;
  dd->mode = NULL;
  dd->metricInfo = agg_metric_info<T>;
  if (device->can_capture) {
    dd->cap = agg_capture<T>;
  } else {
    dd->cap = NULL;
  }
  dd->raster = agg_raster<T>;
#if R_GE_version >= 13
  dd->setPattern      = agg_setPattern<T>;
  dd->releasePattern  = agg_releasePattern<T>;
  dd->setClipPath     = agg_setClipPath<T>;
  dd->releaseClipPath = agg_releaseClipPath<T>;
  dd->setMask         = agg_setMask<T>;
  dd->releaseMask     = agg_releaseMask<T>;
#endif
#if R_GE_version >= 15
  dd->defineGroup     = agg_defineGroup<T>;
  dd->useGroup        = agg_useGroup<T>;
  dd->releaseGroup    = agg_releaseGroup<T>;
  dd->stroke          = agg_stroke<T>;
  dd->fill            = agg_fill<T>;
  dd->fillStroke      = agg_fillStroke<T>;
  dd->capabilities    = agg_capabilities<T>;
#endif
#if R_GE_version >= 16
  dd->glyph           = agg_glyph<T>;
#endif
  // UTF-8 support
  dd->wantSymbolUTF8 = (Rboolean) 1;
  dd->hasTextUTF8 = (Rboolean) 1;
  dd->textUTF8 = agg_text<T>;
  dd->strWidthUTF8 = agg_strwidth<T>;
  
  // Screen Dimensions in pts
  dd->left = 0.0;
  dd->top = 0.0;
  dd->right = device->width;
  dd->bottom = device->height;
  
  // Magic constants copied from other graphics devices
  // nominal character sizes in pts
  dd->cra[0] = 0.9 * device->pointsize * device->res_mod;
  dd->cra[1] = 1.2 * device->pointsize * device->res_mod;
  // character alignment offsets
  dd->xCharOffset = 0.4900;
  dd->yCharOffset = 0.3333;
  dd->yLineBias = 0.2;
  // inches per pt
  dd->ipr[0] = 1.0 / (72 * device->res_mod);
  dd->ipr[1] = 1.0 / (72 * device->res_mod);
  
  // Capabilities
#if R_GE_version >= 15
  dd->capabilities = agg_capabilities<T>;
#endif
  dd->canClip = TRUE;
#if R_GE_version >= 14
  dd->deviceClip = TRUE;
#endif
  dd->canHAdj = 2;
  dd->canChangeGamma = FALSE;
  dd->displayListOn = FALSE;
  dd->haveTransparency = 2;
  dd->haveTransparentBg = 2;
  dd->useRotatedTextInContour =  (Rboolean) 1;
  
#if R_GE_version >= 13
  dd->deviceVersion = 16; //R_GE_glyphs;
#endif
  
  device->device_id = DEVICE_COUNTER++;
  dd->deviceSpecific = device;
  
  return dd;
}

template<class T>
void makeDevice(T* device, const char *name) {
  R_GE_checkVersionOrDie(R_GE_version);
  R_CheckDeviceAvailable();
  BEGIN_SUSPEND_INTERRUPTS {
    pDevDesc dev = agg_device_new<T>(device);
    if (dev == NULL)
      Rf_error("agg device failed to open");
    
    pGEDevDesc dd = GEcreateDevDesc(dev);
    GEaddDevice2(dd, name);
    GEinitDisplayList(dd);
    
  } END_SUSPEND_INTERRUPTS;
}
