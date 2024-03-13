#pragma once

#include "ragg.h"
#include "AggDevice.h"

template<class PIXFMT>
class AggDeviceCapture : public AggDevice<PIXFMT> {
public:
  bool can_capture = true;
  
  AggDeviceCapture(const char* fp, int w, int h, double ps, int bg, double res,
                   double scaling, bool snap) :
    AggDevice<PIXFMT>(fp, w, h, ps, bg, res, scaling, snap)
  {
    
  }
  // Behaviour
  bool savePage() {
    return true;
  }
  SEXP capture() {
    SEXP raster = PROTECT(Rf_allocVector(INTSXP, this->width * this->height));
    agg::rendering_buffer caprbuf(reinterpret_cast<agg::int8u*>(INTEGER(raster)),
                                  this->width, this->height, this->width * 4);
    agg::convert<pixfmt_r_raster, pixfmt_type_32>(&caprbuf, &this->rbuf);
    SEXP dims = PROTECT(Rf_allocVector(INTSXP, 2));
    INTEGER(dims)[0] = this->height;
    INTEGER(dims)[1] = this->width;
    Rf_setAttrib(raster, R_DimSymbol, dims);
    UNPROTECT(2);
    return raster;
  }
};

typedef AggDeviceCapture<pixfmt_type_32> AggDeviceCaptureAlpha;
