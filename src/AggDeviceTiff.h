#include "ragg.h"
#include "AggDevice.h"
#include "AggDevice16.h"

#include <tiffio.h>

template<class PIXFMT>
class AggDeviceTiff : public AggDevice<PIXFMT> {
  int compression;
  int encoding;
public:
  AggDeviceTiff(const char* fp, int w, int h, double ps, int bg, double res, int comp = 0, int enc = 0) : 
    AggDevice<PIXFMT>(fp, w, h, ps, bg, res),
    compression(comp),
    encoding(enc)
  {
    
  }
  
  // Behaviour
  bool savePage() {
    char buf[PATH_MAX+1];
    snprintf(buf, PATH_MAX, this->file.c_str(), this->pageno); buf[PATH_MAX] = '\0';
    TIFF *out= TIFFOpen(buf, "w");
    if (!out) return false;
    
    // Image dims
    TIFFSetField (out, TIFFTAG_IMAGEWIDTH, this->width);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, this->height);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, PIXFMT::num_components);
    if (PIXFMT::num_components == 4) {
      TIFFSetField(out, TIFFTAG_EXTRASAMPLES, EXTRASAMPLE_ASSOCALPHA);
    }
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(out, TIFFTAG_XRESOLUTION, this->res_mod * 72.0);
    TIFFSetField(out, TIFFTAG_YRESOLUTION, this->res_mod * 72.0);
    TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, 2); // Inches
    
    // Compression
    if (compression) TIFFSetField(out, TIFFTAG_COMPRESSION, compression);
    if (encoding) TIFFSetField(out, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
    
    // Required
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    
    // Strip equals scanline
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, this->width * PIXFMT::num_components));
    
    agg::row_ptr_cache<unsigned char> buffer_rows(
        this->buffer, this->width, this->height, this->rbuf.stride_abs()
    );
    
    
    //Now writing image to the file one strip at a time
    for (uint32 row = 0; row < this->height; row++) {
      if (TIFFWriteScanline(out, buffer_rows.row_ptr(row), row, 0) < 0) {
        (void) TIFFClose(out);
        return false;
      }
    }
    
    (void) TIFFClose(out);
    
    return true;
  }
};

typedef AggDeviceTiff<pixfmt_type_24> AggDeviceTiffNoAlpha;
typedef AggDeviceTiff<pixfmt_type_32> AggDeviceTiffAlpha;

// This is more or less a complete copy of the above 8bit implementation, 
// but subclassing AggDevice16. There is probably a better setup

template<class PIXFMT>
class AggDeviceTiff16 : public AggDevice16<PIXFMT> {
  int compression;
  int encoding;
public:
  AggDeviceTiff16(const char* fp, int w, int h, double ps, int bg, double res, int comp = 0, int enc = 0) : 
    AggDevice16<PIXFMT>(fp, w, h, ps, bg, res),
    compression(comp),
    encoding(enc)
  {
    
  }
  
  // Behaviour
  bool savePage() {
    char buf[PATH_MAX+1];
    snprintf(buf, PATH_MAX, this->file.c_str(), this->pageno); buf[PATH_MAX] = '\0';
    TIFF *out= TIFFOpen(buf, "w");
    if (!out) return false;
    
    // Image dims
    TIFFSetField (out, TIFFTAG_IMAGEWIDTH, this->width);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, this->height);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, PIXFMT::num_components);
    if (PIXFMT::num_components == 4) {
      TIFFSetField(out, TIFFTAG_EXTRASAMPLES, EXTRASAMPLE_ASSOCALPHA);
    }
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(out, TIFFTAG_XRESOLUTION, this->res_mod * 72.0);
    TIFFSetField(out, TIFFTAG_YRESOLUTION, this->res_mod * 72.0);
    TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, 2); // Inches
    
    // Compression
    if (compression) TIFFSetField(out, TIFFTAG_COMPRESSION, compression);
    if (encoding) TIFFSetField(out, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
    
    // Required
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    
    // Strip equals scanline
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, this->width * PIXFMT::num_components));
    
    agg::row_ptr_cache<unsigned char> buffer_rows(
        this->buffer, this->width, this->height, this->rbuf.stride_abs()
    );
    
    
    //Now writing image to the file one strip at a time
    for (uint32 row = 0; row < this->height; row++) {
      if (TIFFWriteScanline(out, buffer_rows.row_ptr(row), row, 0) < 0) {
        (void) TIFFClose(out);
        return false;
      }
    }
    
    (void) TIFFClose(out);
    
    return true;
  }
};

typedef AggDeviceTiff16<pixfmt_type_48> AggDeviceTiff16NoAlpha;
typedef AggDeviceTiff16<pixfmt_type_64> AggDeviceTiff16Alpha;

