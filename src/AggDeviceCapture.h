#include "ragg.h"
#include "AggDevice.h"

template<class PIXFMT>
class AggDeviceCapture : public AggDevice<PIXFMT> {
public:
  bool can_capture = true;
  
  AggDeviceCapture(const char* fp, int w, int h, double ps, int bg, double res) :
    AggDevice<PIXFMT>(fp, w, h, ps, bg, res)
  {
    
  }
  // Behaviour
  bool savePage() {
    return true;
  }
  SEXP capture() {
    SEXP raster = PROTECT(Rf_allocVector(INTSXP, this->width * this->height));
    memcpy(INTEGER(raster), this->buffer, this->width * this->height * 4);
    SEXP dims = PROTECT(Rf_allocVector(INTSXP, 2));
    INTEGER(dims)[0] = this->height;
    INTEGER(dims)[1] = this->width;
    Rf_setAttrib(raster, R_DimSymbol, dims);
    UNPROTECT(2);
    return raster;
  }
};

typedef AggDeviceCapture<pixfmt_type_32> AggDeviceCaptureAlpha;
