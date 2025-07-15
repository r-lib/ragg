#pragma once

#include <R_ext/Print.h> 

#include <webp/encode.h>

#include "ragg.h"
#include "AggDevice.h"
#include "files.h"

template<class PIXFMT>
class AggDeviceWebP : public AggDevice<PIXFMT> {
public:
  AggDeviceWebP(const char* fp, int w, int h, double ps, int bg,
                double res, double scaling, bool snap,
                bool lossy, int quality)
    : AggDevice<PIXFMT>(fp, w, h, ps, bg, res, scaling, snap),
      lossy_(lossy), quality_(quality)
  {}

  static int FileWriter(const uint8_t* data, size_t data_size,
                        const WebPPicture* picture) {
    auto fd = static_cast<FILE*>(picture->custom_ptr);
    return fwrite(data, 1, data_size, fd) == data_size;
  }

  bool savePage() {
    char buf[PATH_MAX+1];
    snprintf(buf, PATH_MAX, this->file.c_str(), this->pageno);
    buf[PATH_MAX] = '\0';

    std::unique_ptr<FILE, decltype(&std::fclose)>
      fd(unicode_fopen(buf, "wb"), &std::fclose);
    if (!fd) return false;

    demultiply<PIXFMT>(this->pixf);

    WebPPicture pic;
    if (!WebPPictureInit(&pic)) return false;
    pic.width = this->width;
    pic.height = this->height;
    pic.writer = FileWriter;
    pic.custom_ptr = fd.get();

    WebPConfig config;
    if (!WebPConfigInit(&config)) {
      WebPPictureFree(&pic);
      return false;
    }
    config.quality  = float(quality_);
    config.lossless = lossy_ ? 0 : 1;

    const int stride = this->rbuf.stride_abs();
    const auto importer = (PIXFMT::num_components == 3)
      ? WebPPictureImportRGB
      : WebPPictureImportRGBA;

    if (!importer(&pic,
                  reinterpret_cast<const uint8_t*>(this->buffer),
                  stride)) {
      Rf_warning("WebPPictureImport failed: error code %d",
        pic.error_code);
      WebPPictureFree(&pic);
      return false;
    }

    const bool ok = WebPEncode(&config, &pic);
    if (!ok) 
      Rf_warning("WebPEncode failed: error code %d", pic.error_code);

    WebPPictureFree(&pic);
    return ok;
  }

private:
  bool lossy_;
  int  quality_;
};

typedef AggDeviceWebP<pixfmt_type_24> AggDeviceWebPNoAlpha;
typedef AggDeviceWebP<pixfmt_type_32> AggDeviceWebPAlpha;
