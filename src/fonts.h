#include <string>
#include "ragg.h"

#include <R_ext/Rdynload.h>

#ifndef FONTS_INCLUDED
#define FONTS_INCLUDED

font_map& get_font_map();

static int locate_font(const char *family, int italic, int bold, char *path, int max_path_length) {
  static int (*p_locate_font)(const char *family, int italic, int bold, char *path, int max_path_length) = NULL;
  if (p_locate_font == NULL) {
    p_locate_font = (int(*)(const char *, int, int, char *, int)) R_GetCCallable("systemfonts", "locate_font");
  }
  return p_locate_font(family, italic, bold, path, max_path_length);
}

static std::pair<std::string, int> get_font_file(const char* family, int bold, 
                                                 int italic, int symbol) {
  const char* fontfamily = family;
  if (symbol) {
    fontfamily = "Symbol";
  }
  
  font_key key = std::make_tuple(std::string((char *) fontfamily), bold, italic);
  font_map& map = get_font_map();
  font_map::iterator font_it = map.find(key);
  if (font_it != map.end()) {
    return font_it->second;
  }
  char *path = new char[PATH_MAX+1];
  path[PATH_MAX] = '\0';
  int index = locate_font(fontfamily, italic, bold, path, PATH_MAX);
  std::pair<std::string, int> res;
  res.first = path;
  res.second = index;
  delete[] path;
  
  map[key] = res;
  
  return res;
}

#endif
