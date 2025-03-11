#pragma once

#include "ragg.h"
#include "AggDevice.h"

template<class PIXFMT>
class AggDeviceRecord : public AggDevice<PIXFMT> {
public:
  int width;
  int height;
  AggDeviceRecord(int w, int h, double ps, int bg, double res, double scaling, bool snap) :
  AggDevice<PIXFMT>("", 0, 0, ps, bg, res, scaling, snap),
  width(w),
  height(h)
  {

  }
  SEXP createPattern(SEXP pattern) {
    if (Rf_isNull(pattern)) {
      return Rf_ScalarInteger(-1);
    }
    int key = this->pattern_cache_next_id;
    this->pattern_cache_next_id++;

    return Rf_ScalarInteger(key);
  }
  SEXP createClipPath(SEXP path, SEXP ref) {
    int key;
    if (Rf_isNull(path)) {
      return Rf_ScalarInteger(-1);
    }
    if (Rf_isNull(ref)) {
      key = this->clip_cache_next_id;
      this->clip_cache_next_id++;
    } else {
      key = INTEGER(ref)[0];
    }
    return Rf_ScalarInteger(key);
  }
  SEXP createMask(SEXP mask, SEXP ref) {
    int key;
    if (Rf_isNull(mask)) {
      return Rf_ScalarInteger(-1);
    }
    if (Rf_isNull(ref)) {
      key = this->mask_cache_next_id;
      this->mask_cache_next_id++;
    } else {
      key = INTEGER(ref)[0];
    }
    return Rf_ScalarInteger(key);
  }
  SEXP renderGroup(SEXP source, int op, SEXP destination) {
    int key = this->group_cache_next_id;
    this->group_cache_next_id++;
    return Rf_ScalarInteger(key);
  }
};

typedef AggDeviceRecord<pixfmt_type_32> AggDeviceRecordAlpha;
