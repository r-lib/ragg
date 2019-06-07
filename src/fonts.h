#include <string>
#include "ragg.h"

#ifndef FONTS_INCLUDED
#define FONTS_INCLUDED

font_map* get_font_map();

static std::string get_font_file(const char* family, int bold, int italic, int symbol) {
  const char* fontfamily;
  if (symbol) {
    fontfamily = "Symbol";
  } else {
    fontfamily = *family ? family : "Arial";
  }
  
  font_key key = std::make_tuple(std::string((char *) fontfamily), bold, italic);
  font_map* map = get_font_map();
  font_map::iterator font_it = map->find(key);
  if (font_it != map->end()) {
    return font_it->second;
  }
  char *path = new char[PATH_MAX+1];
  path[PATH_MAX] = '\0';
  (*p_locate_font)(fontfamily, italic, bold, path, PATH_MAX);
  std::string res = path;
  delete[] path;
  
  (*map)[key] = res;
  
  return res;
}

#endif
