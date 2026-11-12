#pragma once

#include "ragg.h"
#include "AggDevice.h"
#include "util/agg_color_conv.h"

/* For htons */
#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

// Functor for dimming alpha if needed
struct AlphaDim {
  double alpha_mod;

  AlphaDim(double alpha_mod) : alpha_mod(alpha_mod) { }

  inline void operator () (agg::rgba16* p)
  {
      p->a *= alpha_mod;
  }
};

template<class PIXFMT>
class AggDevice16 : public AggDevice<PIXFMT, agg::rgba16, pixfmt_type_64> {
public:
  double alpha_mod;

  AggDevice16(const char* fp, int w, int h, double ps, int bg, double res,
              double scaling, bool snap, double alpha_mod = 1.0) :
    AggDevice<PIXFMT, agg::rgba16, pixfmt_type_64>(fp, w, h, ps, bg, res, scaling, snap),
    alpha_mod(alpha_mod)
  {
      this->background = convertColour(this->background_int);
      this->renderer.clear(this->background);
  }

  void to_bigend() {
    uint16_t * buf =  reinterpret_cast<uint16_t *>(this->buffer);
    int64_t n = static_cast<int64_t>(this->width) * this->height * PIXFMT::num_components;
    for (int64_t i = 0; i < n; i++) {
      buf[i] = htons(buf[i]);
    }
  }

private:
  inline agg::rgba16 convertColour(unsigned int col) {
    agg::rgba16 new_col = agg::rgba8(R_RED(col), R_GREEN(col), R_BLUE(col), R_ALPHA(col));
    if (alpha_mod != 1.0 && new_col.a != 65535) new_col.a *= alpha_mod;
    return new_col.premultiply();
  }
  // Mask colours must also be built from 16-bit channel values; the base
  // implementation would interpret the 8-bit channels as 16-bit ones
  inline agg::rgba32 convertMaskCol(unsigned int col) {
    return agg::rgba32(convertColour(col));
  }
};
