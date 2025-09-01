#pragma once

#include <webp/encode.h>
#include <webp/mux.h>

#include <cstring>
#include <vector>
#include <memory>
#include <stdexcept>

#include "AggDevice.h"
#include "files.h"
#include "ragg.h"

using WebPPicturePtr = std::unique_ptr<WebPPicture, void(*)(WebPPicture*)>;
using WebPDataPtr = std::unique_ptr<WebPData, void(*)(WebPData*)>;
using FilePtr = std::unique_ptr<FILE, int(*)(FILE*)>;

inline WebPPicturePtr make_webp_picture() {
  WebPPicture* pic = new WebPPicture;
  if (!WebPPictureInit(pic)) {
    delete pic;
    throw std::runtime_error("WebPPictureInit failed");
  }
  return WebPPicturePtr(pic, WebPPictureFree);
}

inline WebPDataPtr make_webp_data() {
  WebPData* data = new WebPData;
  return WebPDataPtr(data, WebPDataClear);
}

inline FilePtr make_file(const char* filename, const char* mode) {
  FILE* f = unicode_fopen(filename, mode);
  if (!f) {
    throw std::runtime_error("Failed to open file");
  }
  return FilePtr(f, fclose);
}

// A variant of AggDeviceWebP that captures each rendered page to memory,
// encodes it as a WebP frame, and then uses libwebp's mux API to assemble
// them into a single animated WebP on close().
template <class PIXFMT>
class AggDeviceWebPAnim : public AggDevice<PIXFMT> {
 private:
  static constexpr uint32_t RGB_MASK = 0xFFFFFF;

  static const char* webp_error_name(int error_code) {
    static constexpr const char* errors[] = {"OK", "OUT_OF_MEMORY", "BITSTREAM_OUT_OF_MEMORY",
                                             "NULL_PARAMETER", "INVALID_CONFIGURATION", "BAD_DIMENSION",
                                             "PARTITION0_OVERFLOW", "PARTITION_OVERFLOW", "BAD_WRITE",
                                             "FILE_TOO_BIG", "USER_ABORT"};
    constexpr int num_errors = sizeof(errors) / sizeof(errors[0]);
    return (error_code >= 0 && error_code < num_errors) ? errors[error_code] : "UNKNOWN";
  }

  static const char* mux_error_name(WebPMuxError error_code) {
    static constexpr const char* errors[] = {"OK", "NOT_FOUND", "INVALID_ARGUMENT", "BAD_DATA",
                                             "MEMORY_ERROR", "NOT_ENOUGH_DATA"};
    constexpr int num_errors = sizeof(errors) / sizeof(errors[0]);
    return (error_code >= 0 && error_code < num_errors) ? errors[error_code] : "UNKNOWN";
  }

 public:
  AggDeviceWebPAnim(const char* fp, int w, int h, double pointsize,
                    int background, double res, double scaling, bool snap_rect,
                    bool los, int qual, int delay, int n_count)
      : AggDevice<PIXFMT>(fp, w, h, pointsize, background, res, scaling,
                          snap_rect),
        lossy(los),
        quality(qual),
        delay_ms(delay),
        loop_count(n_count),
        mux(WebPMuxNew(), WebPMuxDelete) {
    if (!mux) {
      throw std::runtime_error("Failed to create WebP mux");
    }

    WebPMuxAnimParams params;
    std::memset(&params, 0, sizeof(params));
    params.bgcolor = static_cast<uint32_t>(background) & RGB_MASK;
    params.loop_count = loop_count;

    if (WebPMuxSetAnimationParams(mux.get(), &params) != WEBP_MUX_OK) {
      throw std::runtime_error("Failed to set WebP animation parameters");
    }
  }

  bool savePage() {
    AggDevice<PIXFMT>::savePage();
    demultiply<PIXFMT>(this->pixf);

    try {
      auto pic = make_webp_picture();

      WebPMemoryWriter wr;
      WebPMemoryWriterInit(&wr);
      auto cleanup = [](WebPMemoryWriter* w) { WebPMemoryWriterClear(w); };
      std::unique_ptr<WebPMemoryWriter, decltype(cleanup)> wr_guard(&wr, cleanup);

      pic->width = this->width;
      pic->height = this->height;
      pic->writer = WebPMemoryWrite;
      pic->custom_ptr = &wr;

      WebPConfig config;
      if (!WebPConfigInit(&config)) {
        Rf_warning("WebPConfigInit failed");
        return false;
      }
      config.quality = float(quality);
      config.lossless = lossy ? 0 : 1;

      const int stride = this->rbuf.stride_abs();
      const auto importer = (PIXFMT::num_components == 3) ? WebPPictureImportRGB
                                                          : WebPPictureImportRGBA;

      if (!importer(pic.get(), reinterpret_cast<const uint8_t*>(this->buffer), stride)) {
        Rf_warning("WebPPictureImport failed: %s", webp_error_name(pic->error_code));
        return false;
      }

      if (!WebPEncode(&config, pic.get())) {
        Rf_warning("WebPEncode failed: %s", webp_error_name(pic->error_code));
        return false;
      }

      if (wr.size > 0 && wr.mem != nullptr) {
        frames.emplace_back(wr.mem, wr.mem + wr.size);
      } else {
        Rf_warning("Empty or invalid WebP frame data");
        return false;
      }
      return true;

    } catch (const std::exception& e) {
      Rf_warning("Exception in savePage: %s", e.what());
      return false;
    }
  }

  void close() {
    savePage();

    for (size_t i = 0; i < frames.size(); ++i) {
      const auto& raw = frames[i];
      WebPMuxFrameInfo finfo;
      std::memset(&finfo, 0, sizeof(finfo));
      finfo.bitstream.bytes = raw.data();
      finfo.bitstream.size = raw.size();
      finfo.duration = delay_ms;
      finfo.id = WEBP_CHUNK_ANMF;
      finfo.x_offset = 0;
      finfo.y_offset = 0;
      finfo.dispose_method = WEBP_MUX_DISPOSE_NONE;
      finfo.blend_method = WEBP_MUX_BLEND;
      WebPMuxError frame_err = WebPMuxPushFrame(mux.get(), &finfo, 1);
      if (frame_err != WEBP_MUX_OK) {
        Rf_error("Failed to push WebP frame %zu/%zu: %s (%zu bytes)",
                 i + 1, frames.size(), mux_error_name(frame_err), raw.size());
      }
    }

    try {
      auto out = make_webp_data();
      WebPMuxError mux_err = WebPMuxAssemble(mux.get(), out.get());
      if (mux_err != WEBP_MUX_OK) {
        Rf_error("WebP mux assemble failed: %s (%zu frames)", mux_error_name(mux_err), frames.size());
      }

      auto fd = make_file(this->file.c_str(), "wb");
      if (fwrite(out->bytes, 1, out->size, fd.get()) != out->size) {
        Rf_error("Failed to write WebP animation file");
      }

    } catch (...) {
      AggDevice<PIXFMT>::close();
      throw;
    }

    AggDevice<PIXFMT>::close();
  }

 private:
  const bool lossy;
  const int quality;
  const int delay_ms;
  const int loop_count;
  std::unique_ptr<WebPMux, void(*)(WebPMux*)> mux;
  std::vector<std::vector<uint8_t>> frames;
};

typedef AggDeviceWebPAnim<pixfmt_type_24> AggDeviceWebPAnimNoAlpha;
typedef AggDeviceWebPAnim<pixfmt_type_32> AggDeviceWebPAnimAlpha;

template class AggDeviceWebPAnim<pixfmt_type_24>;
template class AggDeviceWebPAnim<pixfmt_type_32>;
