#pragma once

extern "C" {
#include <png.h>
}

#include "ragg.h"
#include "AggDevice.h"
#include "AggDevice16.h"

template<class PIXFMT>
class AggDevicePng : public AggDevice<PIXFMT> {
public:
  AggDevicePng(const char* fp, int w, int h, double ps, int bg, double res, double scaling) : 
    AggDevice<PIXFMT>(fp, w, h, ps, bg, res, scaling)
  {
    
  }
  
  // Behaviour
  bool savePage() {
    char buf[PATH_MAX+1];
    snprintf(buf, PATH_MAX, this->file.c_str(), this->pageno); buf[PATH_MAX] = '\0';
    FILE* fd = fopen(buf, "wb");
    if(!fd) return false;
    
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) return false;
    
    png_infop info = png_create_info_struct(png);
    if (!info) return false;
    
    if (setjmp(png_jmpbuf(png))) return false;
    
    png_init_io(png, fd);
    
    png_set_IHDR(
      png,
      info,
      this->width, this->height,
      8,
      PIXFMT::num_components == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT
    );
    
    // Write in physical dimensions
    unsigned int ppm = this->res_real / 0.0254;
    png_set_pHYs(png, info, ppm, ppm, 1);
    
    // Write prefered background, just because...
    png_color_16 background;
    background.red = this->background.r;
    background.green = this->background.g;
    background.blue = this->background.b;
    png_set_bKGD(png, info, &background);
    
    png_write_info(png, info);
    
    demultiply<PIXFMT>(this->pixf);
    
    agg::row_ptr_cache<unsigned char> buffer_rows(
        this->buffer, this->width, this->height, this->rbuf.stride_abs()
    );
    
    png_write_image(png, (png_byte **) buffer_rows.rows());
    png_write_end(png, NULL);
    
    png_destroy_write_struct(&png, &info);
    fclose(fd);
    
    return true;
  }
};

typedef AggDevicePng<pixfmt_type_24> AggDevicePngNoAlpha;
typedef AggDevicePng<pixfmt_type_32> AggDevicePngAlpha;

template<class PIXFMT>
class AggDevicePng16 : public AggDevice16<PIXFMT> {
public:
  AggDevicePng16(const char* fp, int w, int h, double ps, int bg, double res, double scaling, double alpha_mod = 1.0) : 
  AggDevice16<PIXFMT>(fp, w, h, ps, bg, res, scaling, alpha_mod)
  {
    
  }
  
  // Behaviour
  bool savePage() {
    char buf[PATH_MAX+1];
    snprintf(buf, PATH_MAX, this->file.c_str(), this->pageno); buf[PATH_MAX] = '\0';
    FILE* fd = fopen(buf, "wb");
    if(!fd) return false;
    
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) return false;
    
    png_infop info = png_create_info_struct(png);
    if (!info) return false;
    
    if (setjmp(png_jmpbuf(png))) return false;
    
    png_init_io(png, fd);
    
    png_set_IHDR(
      png,
      info,
      this->width, this->height,
      16,
      PIXFMT::num_components == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT
    );
    
    // Write in physical dimensions
    unsigned int ppm = this->res_real / 0.0254;
    png_set_pHYs(png, info, ppm, ppm, 1);
    
    // Write prefered background, just because...
    png_color_16 background;
    background.red = this->background.r;
    background.green = this->background.g;
    background.blue = this->background.b;
    png_set_bKGD(png, info, &background);
    
    png_write_info(png, info);
    
    demultiply<PIXFMT>(this->pixf);
    this->to_bigend();
    
    agg::row_ptr_cache<unsigned char> buffer_rows(
        this->buffer, this->width, this->height, this->rbuf.stride_abs()
    );
    png_write_image(png, (png_byte **) buffer_rows.rows());
    png_write_end(png, NULL);
    
    png_destroy_write_struct(&png, &info);
    fclose(fd);
    
    return true;
  }
};

typedef AggDevicePng16<pixfmt_type_48> AggDevicePng16NoAlpha;
typedef AggDevicePng16<pixfmt_type_64> AggDevicePng16Alpha;
