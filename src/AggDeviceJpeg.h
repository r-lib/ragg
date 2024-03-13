#pragma once

#include "ragg.h"
#include "AggDevice.h"
#include "files.h"

#define TRUE 1
#define FALSE 0
#include <jpeglib.h>


template<class PIXFMT>
class AggDeviceJpeg : public AggDevice<PIXFMT> {
  int quality;
  int smoothing;
  int method;
public:
  AggDeviceJpeg(const char* fp, int w, int h, double ps, int bg, double res, double scaling, bool snap, int qual, int smooth, int meth) : 
  AggDevice<PIXFMT>(fp, w, h, ps, bg, res, scaling, snap),
  quality(qual),
  smoothing(smooth),
  method(meth)
  {
    
  }
  // Behaviour
  void newPage(unsigned int bg) {
    if (this->pageno != 0) {
      if (!savePage()) {
        Rf_warning("agg could not write to the given file");
      }
    }
    this->renderer.reset_clipping(true);
    // Fill background with white first to avoid weird transparency issues
    this->renderer.clear(agg::rgba8(255, 255, 255, 255));
    
    if (this->visibleColour(bg)) {
      this->renderer.fill(this->convertColour(bg));
    } else {
      this->renderer.fill(this->background);
    }
    this->pageno++;
  };
  bool savePage() {
    char buf[PATH_MAX+1];
    snprintf(buf, PATH_MAX, this->file.c_str(), this->pageno); buf[PATH_MAX] = '\0';
    FILE* fd = unicode_fopen(buf, "wb");
    if(!fd) return false;
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fd);
    
    cinfo.image_width       = this->width;
    cinfo.image_height      = this->height;
    cinfo.density_unit      = 1;
    cinfo.X_density         = this->res_real;
    cinfo.Y_density         = this->res_real;
    cinfo.input_components  = 3;
    cinfo.in_color_space    = JCS_RGB;
    cinfo.smoothing_factor  = this->smoothing;
    switch (this->method) {
    case 0: cinfo.dct_method = JDCT_ISLOW;
      break;
    case 1: cinfo.dct_method = JDCT_IFAST;
      break;
    case 2: cinfo.dct_method = JDCT_FLOAT;
      break;
    }
    
    jpeg_set_defaults(&cinfo);
    jpeg_set_colorspace(&cinfo, JCS_RGB);
    jpeg_set_quality(&cinfo, this->quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    
    agg::row_ptr_cache<unsigned char> buffer_rows(
        this->buffer, this->width, this->height, this->rbuf.stride_abs()
    );
    
    JSAMPROW row_pointer;
    for (int row = 0; row < this->height; row++) {
      row_pointer = (JSAMPROW) buffer_rows.row_ptr(row);
      jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }
    
    jpeg_finish_compress(&cinfo);
    fclose(fd);
    jpeg_destroy_compress(&cinfo);
    
    return true;
  };
};

typedef AggDeviceJpeg<pixfmt_type_24> AggDeviceJpegNoAlpha;
