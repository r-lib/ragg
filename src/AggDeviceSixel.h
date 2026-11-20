#pragma once

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <string>
#ifdef _WIN32
#  include <io.h>
#else
#  include <unistd.h>
#endif

#include "ragg.h"
#include "AggDevice.h"
#include "sixel.h"

// Write to fd 1 directly rather than via stdio. Referencing the `stdout`
// FILE* global (e.g. through fwrite()) triggers an R CMD check NOTE about
// compiled code writing to stdout, and direct writes bypass stdio buffering
// so no fflush() is needed. Loops to handle partial writes and EINTR.
static bool write_to_stdout(const char* data, size_t len) {
  while (len > 0) {
#ifdef _WIN32
    unsigned int chunk = (unsigned int) std::min(len, (size_t) UINT_MAX);
    int n = _write(1, data, chunk);
#else
    ssize_t n = ::write(STDOUT_FILENO, data, len);
#endif
    if (n < 0) {
      if (errno == EINTR) continue;
      return false;
    }
    data += n;
    len -= n;
  }
  return true;
}

template<class PIXFMT>
class AggDeviceSixel : public AggDevice<PIXFMT> {
public:
  AggDeviceSixel(int w, int h, double ps, int bg, double res, double scaling, bool snap) : 
  AggDevice<PIXFMT>("", w, h, ps, bg, res, scaling, snap)
  {
    
  }
  // Behaviour
  bool savePage() {
    std::string sixel = sixeldev_impl::as_sixel(this->buffer, this->width, this->height);
    sixel.push_back('\n');
    return write_to_stdout(sixel.data(), sixel.size());
  };
};

typedef AggDeviceSixel<pixfmt_type_24> AggDeviceSixelNoAlpha;
