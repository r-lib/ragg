#ifndef TEXTREN_INCLUDED
#define TEXTREN_INCLUDED

#include <vector>
#include <cstdint>
#include <systemfonts.h>
#include <textshaping.h>


#include "ragg.h"

#include "agg_font_freetype.h"

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

class TextRenderer {
  UTF_UCS converter;
  FontSettings last_font;
  agg::glyph_rendering last_gren;
  std::vector<double> x_buffer;
  std::vector<double> y_buffer;
  std::vector<int> id_buffer;
  
public:
  TextRenderer() :
    converter()
  {
    last_gren = agg::glyph_ren_native_mono;
    get_engine().hinting(true);
    get_engine().flip_y(true);
    get_engine().gamma(agg::gamma_power(1.8));
  }
  
  bool load_font(agg::glyph_rendering gren, const char *family, int face, 
                 double size) {
    FontSettings font = get_font_file(family, 
                                      face == 2 || face == 4, 
                                      face == 3 || face == 4,
                                      face == 5);
    if (!(gren == last_gren && 
        font.index == last_font.index &&
        strncmp(font.file, last_font.file, PATH_MAX) == 0)) {
      if (!get_engine().load_font(font.file, font.index, gren)) {
        Rf_warning("Unable to load font: %s", family);
        return false;
      }
      last_gren = gren;
      get_engine().height(size);
    } else if (size != get_engine().height()) {
      get_engine().height(size);
    }
    last_font = font;
    return true;
  }
  
  double get_text_width(const char* string) {
    double width = 0.0;
    int error = ts_string_width(
      string, 
      last_font, 
      get_engine().height(), 
      72.0, 
      1, 
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
    if (glyph) {
      *ascent = (double) -glyph->bounds.y1;
      *descent = (double) glyph->bounds.y2;
      
      *width = glyph->advance_x;
    }
  }
  
  template<typename renderer_solid>
  void plot_text(double x, double y, const char *string, double rot, double hadj, 
                 renderer_solid &ren_solid) {
    agg::scanline_u8 sl;
    agg::rasterizer_scanline_aa<> ras;
    agg::conv_curve<font_manager_type::path_adaptor_type> curves(get_manager().path_adaptor());
    curves.approximation_scale(2.0);
    
    double width = get_text_width(string);
    
    if (width == 0.0) {
      return;
    }
    
    int expected_max = strlen(string) * 16;
    x_buffer.reserve(expected_max);
    y_buffer.reserve(expected_max);
    id_buffer.reserve(expected_max);
    
    int n_glyphs = 0;
    ts_string_shape(
      string, 
      last_font,
      get_engine().height(),
      72.0,
      x_buffer.data(),
      y_buffer.data(),
      id_buffer.data(),
      NULL,
      &n_glyphs,
      expected_max
    );
    
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
    
    for (int i = 0; i < n_glyphs; ++i) {
      const agg::glyph_cache* glyph = get_manager().glyph(id_buffer[i]);
      if (glyph) {
        double x_offset = x_buffer[i] * cos_rot + y_buffer[i] * sin_rot;
        double y_offset = y_buffer[i] * cos_rot + x_buffer[i] * sin_rot;
        get_manager().init_embedded_adaptors(glyph, x + x_offset, y + y_offset);
        switch(glyph->data_type) {
        default: break;
        case agg::glyph_data_gray8:
          agg::render_scanlines(get_manager().gray8_adaptor(), 
                                get_manager().gray8_scanline(), 
                                ren_solid);
          break;
          
        case agg::glyph_data_outline:
          ras.reset();
          ras.add_path(curves);
          agg::render_scanlines(ras, sl, ren_solid);
          break;
        }
      }
    }
    
    if (rot != 0) {
      get_engine().transform(agg::trans_affine());
    }
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
  
  double text_width(const uint32_t* string, int size) {
    double x = 0, y = 0;
    while (*string) {
      const agg::glyph_cache* glyph = get_manager().glyph(*string);
      if (glyph) {
        get_manager().add_kerning(&x, &y);
        // increment pen position
        x += glyph->advance_x;
        y += glyph->advance_y;
      }
      string++;
    }
    return x;
  }
  
  FontSettings get_font_file(const char* family, int bold, int italic, 
                             int symbol) {
    const char* fontfamily = family;
    if (symbol) {
#if defined _WIN32
      fontfamily = "Segoe UI Symbol";
#else
      fontfamily = "Symbol";
#endif
    }
    return locate_font_with_features(fontfamily, italic, bold);
  }
};

#endif
