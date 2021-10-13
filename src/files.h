#pragma once

#include <stdio.h>

#ifdef _WIN32
#include <Rinternals.h>
#include <windows.h>
#endif


// From vroom: https://github.com/r-lib/vroom/blob/master/src/unicode_fopen.h
inline FILE* unicode_fopen(const char* path, const char* mode) {
  FILE* out;
#ifdef _WIN32
  // First conver the mode to the wide equivalent
  // Only usage is 2 characters so max 8 bytes + 2 byte null.
  wchar_t mode_w[10];
  MultiByteToWideChar(CP_UTF8, 0, mode, -1, mode_w, 9);
  
  // Then convert the path
  wchar_t* buf;
  size_t len = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
  if (len <= 0) {
    Rf_error("Cannot convert file to Unicode: %s", path);
  }
  buf = (wchar_t*)R_alloc(len, sizeof(wchar_t));
  if (buf == NULL) {
    Rf_error("Could not allocate buffer of size: %ll", len);
  }
  
  MultiByteToWideChar(CP_UTF8, 0, path, -1, buf, len);
  out = _wfopen(buf, mode_w);
#else
  out = fopen(path, mode);
#endif
  
  return out;
}
