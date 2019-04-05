#include <vector>

#include "ragg.h"

#include "agg_font_freetype.h"
#include "fonts.h"

#ifndef TEXTREN_INCLUDED
#define TEXTREN_INCLUDED

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

#ifndef u_int32_t
typedef uint32_t u_int32_t;
#endif

static const u_int32_t offsetsFromUTF8[6] = {
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
static int u8_toucs(u_int32_t *dest, int sz, const char *src, int srcsz)
{
  u_int32_t ch;
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
  std::vector<u_int32_t> buffer;
  
public:
  UTF_UCS() {
    // Allocate space in buffer
    buffer.resize(1024);
  }
  ~UTF_UCS() {
  }
  u_int32_t * convert(const char * string, int &n_conv) {
    int n_bytes = strlen(string) + 1;
    int max_size = n_bytes * 4;
    if (buffer.size() < max_size) {
      buffer.resize(max_size);
    }
    
    n_conv = u8_toucs(buffer.data(), max_size, string, -1);
    
    return buffer.data();
  }
};

class TextRenderer {
  font_engine_type feng;
  font_manager_type fman;
  UTF_UCS converter;
  std::pair<std::string, int> last_font;
  agg::glyph_rendering last_gren;
  double last_size;
  
public:
  TextRenderer() :
    feng(),
    fman(feng),
    converter()
  {
    last_font = std::make_pair("", -1);
    last_gren = agg::glyph_ren_native_mono;
    last_size = -1.0;
    feng.hinting(true);
    feng.flip_y(true);
    feng.gamma(agg::gamma_power(1.5));
  }
  
  bool load_font(agg::glyph_rendering gren, const char *family, int face, 
                 double size) {
    std::pair<std::string, int> font = get_font_file(family, 
                                                     face == 2 || face == 4, 
                                                     face == 3 || face == 4,
                                                     face == 5);
    if (!(gren == last_gren || 
        font.second == last_font.second || 
        font.first == last_font.first)) {
      if (!feng.load_font(font.first.c_str(), font.second, gren)) {
        Rf_warning("Unable to load font: %s", family);
        return false;
      }
      last_font = font;
      last_gren = gren;
    }
    if (size != last_size) {
      feng.height(size);
      last_size = size;
    }
    
    return true;
  }
  
  double get_text_width(const char* string) {
    int size_out = 0;
    const uint32_t* string_conv = converter.convert(string, size_out);
    return text_width(string_conv, size_out);
  }
  
  void get_char_metric(int c, double *ascent, double *descent, double *width) {
    bool is_unicode = mbcslocale;
    if (c < 0) {
      is_unicode = true;
      c = -c;
    }
    const agg::glyph_cache* glyph = fman.glyph(c);
    
    *ascent = (double) -glyph->bounds.y1;
    *descent = (double) glyph->bounds.y2;
    
    *width = glyph->advance_x;
  }
  
  template<typename renderer_solid>
  void plot_text(double x, double y, const char *string, double rot, double hadj, 
                 renderer_solid &ren_solid) {
    agg::scanline_u8 sl;
    agg::rasterizer_scanline_aa<> ras;
    agg::conv_curve<font_manager_type::path_adaptor_type> curves(fman.path_adaptor());
    curves.approximation_scale(2.0);
    
    int size_out = 0;
    const uint32_t* string_conv = converter.convert(string, size_out);
    double width = text_width(string_conv, size_out);
    
    if (rot != 0) {
      rot = agg::deg2rad(-rot);
      agg::trans_affine mtx;
      mtx *= agg::trans_affine_rotation(rot);
      feng.transform(mtx);
    }
    
    x -= (width * hadj) * cos(rot);
    y -= (width * hadj) * sin(rot);
    
    while (*string_conv) {
      const agg::glyph_cache* glyph = fman.glyph(*string_conv);
      if (glyph) {
        fman.add_kerning(&x, &y);
        fman.init_embedded_adaptors(glyph, x, y);
        switch(glyph->data_type)
        {
        default: break;
        case agg::glyph_data_gray8:
          agg::render_scanlines(fman.gray8_adaptor(), 
                                fman.gray8_scanline(), 
                                ren_solid);
          break;
          
        case agg::glyph_data_outline:
          ras.reset();
          ras.add_path(curves);
          agg::render_scanlines(ras, sl, ren_solid);
          break;
        }
        
        // increment pen position
        x += glyph->advance_x;
        y += glyph->advance_y;
      }
      string_conv++;
    }
    
    if (rot != 0) {
      feng.transform(agg::trans_affine());
    }
  }

private:
  double text_width(const uint32_t* string, int size) {
    double x = 0, y = 0, first_bearing = 0, last_bearing = 0;
    bool first = true;
    while (*string) {
      const agg::glyph_cache* glyph = fman.glyph(*string);
      if (glyph) {
        if (first) {
          first_bearing = glyph->bounds.x1;
          first = false;
        }
        last_bearing = glyph->advance_x - glyph->bounds.x2;
        fman.add_kerning(&x, &y);
        // increment pen position
        x += glyph->advance_x;
        y += glyph->advance_y;
      }
      string++;
    }
    return x - first_bearing - last_bearing;
  }
};

#endif
