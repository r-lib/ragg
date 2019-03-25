#include <string>
#include <R.h>
#include <Rinternals.h>
#include <fontconfig/fontconfig.h>

#ifndef FONTS_INCLUDED
#define FONTS_INCLUDED

static std::pair<std::string, int> get_font_file(const char* family, int bold, 
                                                 int italic, int symbol) {
  std::pair<std::string, int> res;
  res.first = "";
  res.second = 0;
  
  if (!FcInit()) {
    Rf_warning("Could not initialise fontconfig. Using system font");
    return res;
  }
  FcPattern* pattern;
  
  FcChar8* fontfamily;
  if (symbol) {
    fontfamily = (FcChar8 *) "Symbol";
  } else {
    fontfamily = (FcChar8 *) (*family ? family : "Arial");
  }
  
  if(!(pattern = FcNameParse(fontfamily))) { // Defaults to Arial
    Rf_warning("Fontconfig error: unable to parse font name: %s. Using system font", 
               fontfamily);
    return res;
  }
  FcPatternAddInteger(pattern, FC_WEIGHT, bold ? FC_WEIGHT_BOLD : FC_WEIGHT_MEDIUM);
  FcPatternAddInteger(pattern, FC_SLANT, italic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);
  FcDefaultSubstitute(pattern);
  FcConfigSubstitute(0, pattern, FcMatchPattern);
  
  // Need to initialise result for fontconfig versions prior to 2.10
  // (e.g. old Linux distributions)
  FcResult result = FcResultMatch;
  FcPattern* match = FcFontMatch(0, pattern, &result);
  FcPatternDestroy(pattern);
  
  if (!(match && result == FcResultMatch)) {
    Rf_warning("Fontconfig error: unable to match %s. Using system font", family);
    return res;
  }
  
  FcChar8 *matched_file;
  if (match && FcPatternGetString(match, FC_FILE, 0, &matched_file) == FcResultMatch) {
    res.first = (const char*) matched_file;
  }
  
  FcPatternGetInteger(match, FC_INDEX, 0, &res.second);
  
  FcPatternDestroy(match);
  
  return res;
}

#endif
