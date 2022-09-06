#pragma once

#include <vector>
#include <cstdint>
#include <systemfonts.h>
#include <textshaping.h>


#include "ragg.h"
#include "rendering.h"

#include "agg_font_freetype.h"
#include "agg_span_interpolator_linear.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_allocator.h"
#include "agg_path_storage.h"

#include "util/agg_color_conv.h"

typedef agg::font_engine_freetype_int32 font_engine_type;
typedef agg::font_cache_manager<font_engine_type> font_manager_type;

/*
 Basic UTF-8 manipulation routines
 by Jeff Bezanson
 placed in the public domain Fall 2005
 
 This code is designed to provide the utilities you need to manipulate
 UTF-8 as an internal string encoding. These functions do not perform the
 error checking normally needed when handling UTF-8 data, so if you happen
 to be from the Unicode Consortium you will want to flay me alive.
 I do this because error checking can be performed at the boundaries (I/O),
 with these routines reserved for higher performance on data known to be
 valid.
 
 Source: https://www.cprogramming.com/tutorial/utf8.c
 
 Modified 2019 by Thomas Lin Pedersen to work with const char*
 */

static const uint32_t offsetsFromUTF8[6] = {
  0x00000000UL, 0x00003080UL, 0x000E2080UL,
  0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

static const char trailingBytesForUTF8[256] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/* conversions without error checking
 only works for valid UTF-8, i.e. no 5- or 6-byte sequences
 srcsz = source size in bytes, or -1 if 0-terminated
 sz = dest size in # of wide characters
 
 returns # characters converted
 dest will always be L'\0'-terminated, even if there isn't enough room
 for all the characters.
 if sz = srcsz+1 (i.e. 4*srcsz+4 bytes), there will always be enough space.
 */
static int u8_toucs(uint32_t *dest, int sz, const char *src, int srcsz)
{
  uint32_t ch;
  const char *src_end = src + srcsz;
  int nb;
  int i=0;
  
  while (i < sz-1) {
    nb = trailingBytesForUTF8[(unsigned char)*src];
    if (srcsz == -1) {
      if (*src == 0)
        goto done_toucs;
    }
    else {
      if (src + nb >= src_end)
        goto done_toucs;
    }
    ch = 0;
    switch (nb) {
    /* these fall through deliberately */
    case 5: ch += (unsigned char)*src++; ch <<= 6;
    case 4: ch += (unsigned char)*src++; ch <<= 6;
    case 3: ch += (unsigned char)*src++; ch <<= 6;
    case 2: ch += (unsigned char)*src++; ch <<= 6;
    case 1: ch += (unsigned char)*src++; ch <<= 6;
    case 0: ch += (unsigned char)*src++;
    }
    ch -= offsetsFromUTF8[nb];
    dest[i++] = ch;
  }
  done_toucs:
    dest[i] = 0;
  return i;
}

/*
 End of Basic UTF-8 manipulation routines
 by Jeff Bezanson
*/

class UTF_UCS {
  std::vector<uint32_t> buffer;
  
public:
  UTF_UCS() {
    // Allocate space in buffer
    buffer.resize(1024);
  }
  ~UTF_UCS() {
  }
  uint32_t * convert(const char * string, int &n_conv) {
    int n_bytes = strlen(string) + 1;
    unsigned int max_size = n_bytes * 4;
    if (buffer.size() < max_size) {
      buffer.resize(max_size);
    }
    
    n_conv = u8_toucs(buffer.data(), max_size, string, -1);
    
    return buffer.data();
  }
};

template<typename PIXFMT>
class TextRenderer {
  UTF_UCS converter;
  FontSettings last_font;
  agg::glyph_rendering last_gren;
  std::vector<textshaping::Point> loc_buffer;
  std::vector<uint32_t> id_buffer;
  std::vector<int> cluster_buffer;
  std::vector<unsigned int> font_buffer;
  std::vector<FontSettings> fallback_buffer;
  std::vector<double> scaling_buffer;
  double current_font_height;
  double current_font_size;
  bool no_bearings;
  
public:
  TextRenderer() :
    converter()
  {
    last_gren = agg::glyph_ren_native_mono;
    get_engine().hinting(true);
    get_engine().flip_y(true);
    get_engine().gamma(agg::gamma_power(1.6));
  }
  
  bool load_font(agg::glyph_rendering gren, const char *family, int face, 
                 double size, unsigned int id = 0) {
    FontSettings font = get_font_file(family, 
                                      face == 2 || face == 4, 
                                      face == 3 || face == 4,
                                      face == 5);
    current_font_size = size;
    if (!load_font_from_file(font, gren, size, id)) {
      Rf_warning("Unable to load font: %s", family);
      current_font_height = 0;
      return false;
    }
    current_font_height = size;
    no_bearings = false;
#if defined(_WIN32)
    // Windows emojis have weird right bearings
    if (strcmp("Segoe UI Emoji", get_engine().family()) == 0) {
      no_bearings = true;
    }
#endif
    return true;
  }
  
  double get_text_width(const char* string) {
    double width = 0.0;
    int error = textshaping::string_width(
      string, 
      last_font, 
      current_font_size, 
      72.0, 
      no_bearings ? 0 : 1, 
      &width
    );
    if (error) {
      return 0.0;
    }
    return width;
  }
  
  void get_char_metric(int c, double *ascent, double *descent, double *width) {
    unsigned index = get_engine().get_glyph_index(c);
    const agg::glyph_cache* glyph = get_manager().glyph(index);
    
    // This might also be relevant to non-colour fonts that are unscalable
    double mod = current_font_height / get_engine().height();
    
    // Only use 77 glyph if found and not colour font
    // Last point is to guard against wrong line-heights based in M char in emoji fonts
    if (glyph && !(c == 77 && (index == 0 || glyph->data_type == agg::glyph_data_color))) {
      *ascent = mod * (double) -glyph->bounds.y1;
      *descent = mod * (double) glyph->bounds.y2;
      
      *width = mod * glyph->advance_x;
      
#if defined(__APPLE__)
      // Apple emojis have no descender
      if (glyph->data_type == agg::glyph_data_color && strcmp("Apple Color Emoji", get_engine().family()) == 0) {
        double y_shift = double(glyph->bounds.y1 - glyph->bounds.y2) * 0.1;
        *descent += y_shift;
        *ascent += y_shift;
      }
#endif
    } else {
      // Use global font metrics
      *ascent = mod * get_engine().ascent();
      *descent = mod * get_engine().descent();
      
      *width = mod * get_engine().max_advance();
    }
  }
  
  template<typename TARGET, typename renderer_solid, typename renderer, typename raster, typename scanline>
  void plot_text(double x, double y, const char *string, double rot, double hadj, 
                 renderer_solid &ren_solid, renderer &ren, scanline &sl, unsigned int id,
                 raster &ras_clip, bool clip, agg::path_storage* recording_clip) {
    agg::rasterizer_scanline_aa<> ras;
    agg::conv_curve<font_manager_type::path_adaptor_type> curves(get_manager().path_adaptor());
    curves.approximation_scale(2.0);
    
    double width = get_text_width(string);
    
    if (width == 0.0) {
      return;
    }
    
    int expected_max = strlen(string) * 16;
    if (expected_max == 0) {
      return;
    }
    
    loc_buffer.reserve(expected_max);
    id_buffer.reserve(expected_max);
    cluster_buffer.reserve(expected_max);
    font_buffer.reserve(expected_max);
    fallback_buffer.reserve(expected_max);
    scaling_buffer.reserve(expected_max);
    
    int err = textshaping::string_shape(
      string,
      last_font,
      current_font_size, 
      72.0,
      loc_buffer,
      id_buffer,
      cluster_buffer,
      font_buffer,
      fallback_buffer,
      scaling_buffer
    );
    
    if (err != 0) {
      Rf_warning("textshaping failed to shape the string");
      return;
    }
    
    int n_glyphs = loc_buffer.size();
    
    if (n_glyphs == 0) {
      return;
    }
    
    if (rot != 0) {
      rot = agg::deg2rad(-rot);
      agg::trans_affine mtx;
      mtx *= agg::trans_affine_rotation(rot);
      get_engine().transform(mtx);
    }
    
    double cos_rot = cos(rot);
    double sin_rot = sin(rot);
    
    x -= (width * hadj) * cos_rot;
    y -= (width * hadj) * sin_rot;
    
    // Snap to pixel grid for vertical or horizontal text
    if (fmod(rot, 180) < 1e-6) {
      y = std::round(y);
    } else if (fmod(rot + 90, 180) < 1e-6) {
      x = std::round(x);
    }
    int text_run_start = 0;
    for (int j = 1; j <= n_glyphs; ++j) {
      if (j == n_glyphs || font_buffer[j] != font_buffer[j - 1]) {
        if (fallback_buffer.size() == 0 || // To guard against old textshaping version/solaris mock
            load_font_from_file(fallback_buffer[font_buffer[text_run_start]], last_gren, current_font_size, id)) {
          for (int i = text_run_start; i < j; ++i) {
            const agg::glyph_cache* glyph = get_manager().glyph(id_buffer[i]);
            if (glyph) {
              double x_offset = loc_buffer[i].x * cos_rot + loc_buffer[i].y * sin_rot;
              double y_offset = loc_buffer[i].y * cos_rot + loc_buffer[i].x * sin_rot;
              get_manager().init_embedded_adaptors(glyph, x + x_offset, y + y_offset);
              switch(glyph->data_type) {
              default: break;
              case agg::glyph_data_gray8:
                render<agg::scanline_u8>(get_manager().gray8_adaptor(), ras_clip, 
                                         sl, ren_solid, 
                                         clip);
                break;
                
              case agg::glyph_data_color:
                renderColourGlyph<TARGET>(glyph, x + x_offset, y + y_offset, rot, ren, sl, scaling_buffer[font_buffer[text_run_start]], ras_clip, clip);
                break;
                
              case agg::glyph_data_outline:
                if (recording_clip != NULL) {
                  recording_clip->concat_path(curves);
                  break;
                }
                ras.reset();
                ras.add_path(curves);
                render<agg::scanline_u8>(ras, ras_clip, sl, ren_solid, clip);
                break;
              }
            }
          }
        }
        text_run_start = j;
      }
    }
    
    if (rot != 0) {
      get_engine().transform(agg::trans_affine());
    }
  }

  template<typename TARGET, typename renderer_solid, typename renderer, typename raster, typename scanline>
  void layout_text(double x, double y, SEXP span, double w,
                   renderer_solid &ren_solid, renderer &ren, 
                   scanline &sl, unsigned int id,
                   raster &ras_clip, bool clip, 
                   agg::path_storage* recording_clip) {

    double rot = 0.0;

    /* Shifted from typsetText() because more than one load_font()
     * to do, so that happens here rather than in typesetText() */
    agg::glyph_rendering gren = std::fmod(rot, 90) == 0.0 && recording_clip == NULL ? agg::glyph_ren_agg_gray8 : agg::glyph_ren_outline;

    agg::rasterizer_scanline_aa<> ras;
    agg::conv_curve<font_manager_type::path_adaptor_type> curves(get_manager().path_adaptor());
    curves.approximation_scale(2.0);

    SEXP text = R_GE_spanText(span);
    SEXP family = R_GE_spanFamily(span);
    SEXP weight = R_GE_spanWeight(span);
    SEXP style = R_GE_spanStyle(span);
    double size = 12;

    int n = LENGTH(text);
    int i;
    
    /* Generate some default (plain) arguments */
    SEXP group;
    group = PROTECT(Rf_allocVector(INTSXP, n));
    for (i=0; i<n; i++) { INTEGER(group)[i] = 1; }
    
    /* Generate vectors of italic and bold */
    SEXP italic, bold;
    italic = PROTECT(Rf_allocVector(LGLSXP, n));
    bold = PROTECT(Rf_allocVector(LGLSXP, n));
    for (i=0; i<n; i++) {
        if (REAL(weight)[i] <= 400) {
            LOGICAL(bold)[i] = FALSE;
        } else {
            LOGICAL(bold)[i] = TRUE;
        }
        if (INTEGER(style)[i] != R_GE_text_style_normal) {
            LOGICAL(italic)[i] = FALSE;
        } else {
            LOGICAL(italic)[i] = TRUE;
        }
    }

    /* 'width' arg */
    SEXP width = PROTECT(Rf_allocVector(REALSXP, 1));
    REAL(width)[0] = w;

    /* Call R function from 'textshaping' for now 
     * (this should become call to 'textshaping' C API) */
    SEXP s, t, result;
    t = s = PROTECT(Rf_allocList(7));
    SET_TYPEOF(s, LANGSXP);
    SETCAR(t, Rf_install("shape_text")); t = CDR(t);
    SETCAR(t, text); t = CDR(t);
    SETCAR(t, group); t = CDR(t);
    SETCAR(t, family); t = CDR(t);
    SETCAR(t, italic); t = CDR(t);
    SETCAR(t, bold); t = CDR(t);
    SETCAR(t, width); SET_TAG(t, Rf_install("width"));
    result = PROTECT(Rf_eval(s, R_GlobalEnv));
    UNPROTECT(1); /* t/s */
      
    /* No longer need args (just need result) */
    UNPROTECT(4);
    
    SEXP info = VECTOR_ELT(result, 0);
    SEXP x_offset = VECTOR_ELT(info, 4);
    SEXP y_offset = VECTOR_ELT(info, 5);
    SEXP glyph_id = VECTOR_ELT(info, 1);
    SEXP string_id = VECTOR_ELT(info, 3);

    SEXP metric = VECTOR_ELT(result, 1);
    SEXP height = VECTOR_ELT(metric, 8);
    
    int n_glyphs = LENGTH(VECTOR_ELT(info, 0));
    if (n_glyphs == 0) {
      return;
    }
    
    /* Generate vectors of font info */
    std::vector<FontSettings> fonts;
    std::vector<std::string> families;
    std::vector<int> faces;
    for (i=0; i<n_glyphs; i++) {
        int stringIndex = INTEGER(string_id)[i] - 1;
        families.push_back(CHAR(STRING_ELT(family, stringIndex)));
        if (REAL(weight)[stringIndex] > 400) {
            if (INTEGER(style)[stringIndex] != R_GE_text_style_normal) {
                faces.push_back(4);
            } else {
                faces.push_back(2);
            }
        } else {
            if (INTEGER(style)[stringIndex] != R_GE_text_style_normal) {
                faces.push_back(3);
            } else {
                faces.push_back(1);
            }      
        }
        fonts.push_back(locate_font_with_features(families[i].c_str(), 
                                                  faces[i] == 2 || 
                                                  faces[i] == 4, 
                                                  faces[i] == 3 || 
                                                  faces[i] == 4));
    }

    int text_run_start = 0;
    for (int j = 1; j <= n_glyphs; ++j) {
        std::string font1 = fonts[j].file;
        std::string font2 = fonts[j - 1].file;
        if (j == n_glyphs || font1 != font2) {
            /* There is something going on here:  need to both load_font()
             * and load_font_from_file() to get my test example to work,
             * but not sure why */
            if (!load_font(gren, families[text_run_start].c_str(), 
                           faces[text_run_start], size, id)) 
                return;
            if (load_font_from_file(fonts[text_run_start], 
                                    last_gren, size, id)) {
                for (int i = text_run_start; i < j; ++i) {
                    const agg::glyph_cache* 
                        glyph = get_manager().glyph(INTEGER(glyph_id)[i]);
                    if (glyph) {
                        double x_off = REAL(x_offset)[i];
                        double y_off = REAL(y_offset)[i];
                        double h = REAL(height)[0];
                        get_manager().init_embedded_adaptors(glyph, 
                                                             x + x_off, 
                                                             y + h - y_off);
                        switch(glyph->data_type) {
                        default: break;
                        case agg::glyph_data_gray8:
                            render<agg::scanline_u8>(get_manager().gray8_adaptor(), 
                                                     ras_clip, sl, ren_solid, 
                                                     clip);
                            break;
                
                        case agg::glyph_data_color:
                            renderColourGlyph<TARGET>(glyph, 
                                                      x + x_off, 
                                                      y + h - y_off, 
                                                      0.0, // rot
                                                      ren, sl, 
                                                      1.0, // scaling
                                                      ras_clip, clip);
                            break;
                
                        case agg::glyph_data_outline:
                            if (recording_clip != NULL) {
                                recording_clip->concat_path(curves);
                                break;
                            }
                            ras.reset();
                            ras.add_path(curves);
                            render<agg::scanline_u8>(ras, ras_clip, sl, 
                                                     ren_solid, clip);
                            break;
                        }
                    }
                }
            }
            text_run_start = j;
        }
    }
    UNPROTECT(1);  /* result */
    
  }

  template<typename TARGET, typename renderer_solid, typename renderer, typename raster, typename scanline>
  void render_glyphs(double x, double y, SEXP glyph,
                     renderer_solid &ren_solid, renderer &ren, 
                     scanline &sl, unsigned int id,
                     raster &ras_clip, bool clip, 
                     agg::path_storage* recording_clip) {

    double rot = 0.0;

    /* Shifted from renderText() because more than one load_font()
     * to do, so that happens here rather than in typesetText() */
    agg::glyph_rendering gren = std::fmod(rot, 90) == 0.0 && recording_clip == NULL ? agg::glyph_ren_agg_gray8 : agg::glyph_ren_outline;

    agg::rasterizer_scanline_aa<> ras;
    agg::conv_curve<font_manager_type::path_adaptor_type> curves(get_manager().path_adaptor());
    curves.approximation_scale(2.0);

    SEXP character = R_GE_glyphGlyph(glyph); 
    SEXP font = R_GE_glyphFont(glyph);
    SEXP glyph_id = R_GE_glyphIndex(glyph);
    SEXP x_offset = R_GE_glyphXOffset(glyph);
    SEXP y_offset = R_GE_glyphYOffset(glyph);

    int n_glyphs = LENGTH(character);
    int i;
    
    /* Generate vectors of font info */
    std::vector<FontSettings> fonts;
    std::vector<std::string> families;
    std::vector<int> faces;
    std::vector<double> sizes;
    for (i=0; i<n_glyphs; i++) {
        SEXP f = VECTOR_ELT(font, i);
        SEXP family = R_GE_fontFamily(f);
        SEXP weight = R_GE_fontWeight(f);
        SEXP style = R_GE_fontStyle(f);
        SEXP size = R_GE_fontSize(f);
        families.push_back(CHAR(STRING_ELT(family, 0)));
        if (REAL(weight)[0] > 400) {
            if (INTEGER(style)[0] != R_GE_text_style_normal) {
                faces.push_back(4);
            } else {
                faces.push_back(2);
            }
        } else {
            if (INTEGER(style)[0] != R_GE_text_style_normal) {
                faces.push_back(3);
            } else {
                faces.push_back(1);
            }      
        }
        fonts.push_back(locate_font_with_features(families[i].c_str(), 
                                                  faces[i] == 2 || 
                                                  faces[i] == 4, 
                                                  faces[i] == 3 || 
                                                  faces[i] == 4));
        sizes.push_back(REAL(size)[0]);
    }

    int text_run_start = 0;
    for (int j = 1; j <= n_glyphs; ++j) {
        std::string font1 = fonts[j].file;
        std::string font2 = fonts[j - 1].file;
        if (j == n_glyphs || font1 != font2) {
            /* There is something going on here:  need to both load_font()
             * and load_font_from_file() to get my test example to work,
             * but not sure why */
            if (!load_font(gren, families[text_run_start].c_str(), 
                           faces[text_run_start], sizes[text_run_start], id)) 
                return;
            if (load_font_from_file(fonts[text_run_start], 
                                    last_gren, sizes[text_run_start], id)) {
                for (int i = text_run_start; i < j; ++i) {
                    const agg::glyph_cache* 
                        glyph = get_manager().glyph(INTEGER(glyph_id)[i]);
                    if (glyph) {
                        double x_off = REAL(x_offset)[i];
                        double y_off = REAL(y_offset)[i];
                        get_manager().init_embedded_adaptors(glyph, 
                                                             x + x_off, 
                                                             y + y_off);
                        switch(glyph->data_type) {
                        default: break;
                        case agg::glyph_data_gray8:
                            render<agg::scanline_u8>(get_manager().gray8_adaptor(), 
                                                     ras_clip, sl, ren_solid, 
                                                     clip);
                            break;
                
                        case agg::glyph_data_color:
                            renderColourGlyph<TARGET>(glyph, 
                                                      x + x_off, y + y_off, 
                                                      0.0, // rot
                                                      ren, sl, 
                                                      1.0, // scaling
                                                      ras_clip, clip);
                            break;
                
                        case agg::glyph_data_outline:
                            if (recording_clip != NULL) {
                                recording_clip->concat_path(curves);
                                break;
                            }
                            ras.reset();
                            ras.add_path(curves);
                            render<agg::scanline_u8>(ras, ras_clip, sl, 
                                                     ren_solid, clip);
                            break;
                        }
                    }
                }
            }
            text_run_start = j;
        }
    }
    UNPROTECT(1);  /* result */
    
  }

private:
  inline font_engine_type& get_engine() {
    static font_engine_type engine;
    return engine;
  }
  
  inline font_manager_type& get_manager() {
    static font_manager_type manager(get_engine());
    return manager;
  }
  
  FontSettings get_font_file(const char* family, int bold, int italic, 
                             int symbol) {
    const char* fontfamily = family;
    if (symbol) {
      fontfamily = "symbol";
    }
    return locate_font_with_features(fontfamily, italic, bold);
  }
  
  bool load_font_from_file(FontSettings font, agg::glyph_rendering gren, double size,
                           unsigned int id) {
    if (id != get_engine().id() ||
        !(gren == last_gren && 
        font.index == last_font.index &&
        strncmp(font.file, last_font.file, PATH_MAX) == 0)) {
      if (!get_engine().load_font(font.file, font.index, gren)) {
        return false;
      }
      last_gren = gren;
      get_engine().height(size);
      get_engine().id(id);
    } else if (size != get_engine().height()) {
      get_engine().height(size);
    }
    last_font = font;
    return true;
  } 
  
  template<typename TARGET, typename ren, typename raster, typename scanline>
  void renderColourGlyph(const agg::glyph_cache* glyph, double x, double y, 
                         double rot, ren &renderer, scanline &sl, double scaling, raster &ras_clip, 
                         bool clip) {
    int w = glyph->bounds.x2 - glyph->bounds.x1;
    int h = glyph->bounds.y1 - glyph->bounds.y2;
    agg::rendering_buffer rbuf(glyph->data, w, h, w * 4);
    
    agg::trans_affine img_mtx;
    img_mtx *= agg::trans_affine_translation(0, -glyph->bounds.y1);
    if (scaling > 0) {
      img_mtx *= agg::trans_affine_translation(-double(w)/2, 0);
      img_mtx *= agg::trans_affine_scaling(scaling);
      img_mtx *= agg::trans_affine_translation(scaling * double(w)/2, 0);
    }
#if defined(__APPLE__)
    // Apple emojis have no descender
    if (strcmp("Apple Color Emoji", get_engine().family()) == 0) {
      if (scaling < 0) scaling = 1.0; //shouldn't happen as Apple emojis are not scalable
      img_mtx *= agg::trans_affine_translation(0, scaling * double(h) * 0.12);
    }
#endif
    img_mtx *= agg::trans_affine_rotation(rot);
    img_mtx *= agg::trans_affine_translation(x, y);
    agg::trans_affine src_mtx = img_mtx;
    img_mtx.invert();
    
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator(img_mtx);
    
    agg::rasterizer_scanline_aa<> ras;
    
    agg::path_storage rect;
    rect.remove_all();
    rect.move_to(0, 0);
    rect.line_to(0, h);
    rect.line_to(w, h);
    rect.line_to(w, 0);
    rect.close_polygon();
    agg::conv_transform<agg::path_storage> tr(rect, src_mtx);
    ras.add_path(tr);
    bool interpolate = scaling >= 1 || scaling < 0;
    
    render_raster<pixfmt_col_glyph, TARGET>(rbuf, w, h, ras, ras_clip, sl, interpolator, renderer, interpolate, clip, !interpolate);
  }
};
