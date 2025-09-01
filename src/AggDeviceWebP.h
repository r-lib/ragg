#pragma once

#include <R_ext/Print.h>

#include <webp/encode.h>

#include "ragg.h"
#include "AggDevice.h"
#include "files.h"

template<class PIXFMT>
class AggDeviceWebP : public AggDevice<PIXFMT> {
 private:
  static const char* webp_error_name(int error_code) {
    static constexpr const char* errors[] = {"OK", "OUT_OF_MEMORY", "BITSTREAM_OUT_OF_MEMORY",
                                             "NULL_PARAMETER", "INVALID_CONFIGURATION", "BAD_DIMENSION",
                                             "PARTITION0_OVERFLOW", "PARTITION_OVERFLOW", "BAD_WRITE",
                                             "FILE_TOO_BIG", "USER_ABORT"};
    constexpr int num_errors = sizeof(errors) / sizeof(errors[0]);
    return (error_code >= 0 && error_code < num_errors) ? errors[error_code] : "UNKNOWN";
  }

 public:
  AggDeviceWebP(const char* fp, int w, int h, double ps, int bg,
                double res, double scaling, bool snap,
                bool los, int qual)
    : AggDevice<PIXFMT>(fp, w, h, ps, bg, res, scaling, snap),
      lossy(los), quality(qual)
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

    auto fd = std::unique_ptr<FILE, decltype(&std::fclose)>(
        unicode_fopen(buf, "wb"), &std::fclose);
    if (!fd) return false;

    demultiply<PIXFMT>(this->pixf);

    WebPPicture pic;
    if (!WebPPictureInit(&pic)) return false;

    auto pic_guard = std::unique_ptr<WebPPicture, void(*)(WebPPicture*)>(
        &pic, [](WebPPicture* p) { WebPPictureFree(p); });

    pic.width = this->width;
    pic.height = this->height;
    pic.writer = FileWriter;
    pic.custom_ptr = fd.get();

    WebPConfig config;
    if (!WebPConfigInit(&config)) return false;

    config.quality = float(quality);
    config.lossless = lossy ? 0 : 1;

    const int stride = this->rbuf.stride_abs();
    constexpr auto importer = (PIXFMT::num_components == 3)
        ? WebPPictureImportRGB : WebPPictureImportRGBA;

    if (!importer(&pic, reinterpret_cast<const uint8_t*>(this->buffer), stride)) {
      Rf_warning("WebPPictureImport failed: %s", webp_error_name(pic.error_code));
      return false;
    }

    if (!WebPEncode(&config, &pic)) {
      Rf_warning("WebPEncode failed: %s", webp_error_name(pic.error_code));
      return false;
    }

    return true;
  }

private:
  bool lossy;
  int  quality;
};

typedef AggDeviceWebP<pixfmt_type_24> AggDeviceWebPNoAlpha;
typedef AggDeviceWebP<pixfmt_type_32> AggDeviceWebPAlpha;
