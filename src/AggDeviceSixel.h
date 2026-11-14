#pragma once

#include "ragg.h"
#include "AggDevice.h"
#include "files.h"
#include "sixel.h"

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
    fwrite(sixel.c_str(), 1, sixel.size(), stdout);
    fflush(stdout);
    return true;
  };
};

typedef AggDeviceSixel<pixfmt_type_24> AggDeviceSixelNoAlpha;
