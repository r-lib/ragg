#pragma once

#include "ragg.h"
#include "AggDevice.h"
#include "files.h"

#define TRUE 1
#define FALSE 0
#include <jpeglib.h>
#include <csetjmp>

// libjpeg's default error handler calls exit() which would kill the R
// session. Longjmp back to savePage() instead so we can clean up and fail.
struct ragg_jpeg_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};
extern "C" inline void ragg_jpeg_error_exit(j_common_ptr cinfo) {
  ragg_jpeg_error_mgr* err = reinterpret_cast<ragg_jpeg_error_mgr*>(cinfo->err);
  longjmp(err->setjmp_buffer, 1);
}


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
    struct ragg_jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = ragg_jpeg_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
      // A libjpeg error longjmps here
      jpeg_destroy_compress(&cinfo);
      fclose(fd);
      return false;
    }
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fd);

    cinfo.image_width       = this->width;
    cinfo.image_height      = this->height;
    cinfo.input_components  = 3;
    cinfo.in_color_space    = JCS_RGB;

    // jpeg_set_defaults() resets smoothing_factor, dct_method, and the
    // density fields, so it must be called before they are set
    jpeg_set_defaults(&cinfo);
    jpeg_set_colorspace(&cinfo, JCS_RGB);
    cinfo.density_unit      = 1;
    // Density fields are 16-bit unsigned
    cinfo.X_density         = this->res_real > 65535.0 ? 65535 : (UINT16) this->res_real;
    cinfo.Y_density         = this->res_real > 65535.0 ? 65535 : (UINT16) this->res_real;
    cinfo.smoothing_factor  = this->smoothing;
    switch (this->method) {
    case 0: cinfo.dct_method = JDCT_ISLOW;
      break;
    case 1: cinfo.dct_method = JDCT_IFAST;
      break;
    case 2: cinfo.dct_method = JDCT_FLOAT;
      break;
    }

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
