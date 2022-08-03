#pragma once

// TODO: Consider if main render buffer should be a RenderBuffer object instead
// of defined in the AggDevice class.

#include "ragg.h"
#include "agg_alpha_mask_u8.h"
#include "agg_pixfmt_gray.h"
#include "agg_scanline_u.h"
#include "util/agg_color_conv.h"

template<class PIXFMT>
class RenderBuffer {
public:
  typedef PIXFMT pixfmt_type;
  typedef agg::renderer_base<PIXFMT> renbase_type;
  typedef agg::renderer_scanline_aa_solid<renbase_type> rensolid_type;
  
  int width;
  int height;
  
protected:
  unsigned char* buffer;
  agg::rendering_buffer rbuf;
  pixfmt_type* pixf;
  renbase_type renderer;
  rensolid_type renderer_solid;
  
public:
  RenderBuffer() :
  width(0),
  height(0),
  rbuf()
  {
    buffer = new unsigned char[0];
    rbuf.attach(buffer, 0, 0, 0);
    pixf = new pixfmt_type(rbuf);
    renderer = renbase_type(*pixf);
    renderer_solid = rensolid_type(renderer);
  }
  template<class COLOR>
  RenderBuffer(int _width, int _height, COLOR bg) : 
  width(_width),
  height(_height),
  rbuf()
  {
    buffer = new unsigned char[width * height * PIXFMT::pix_width];
    rbuf.attach(buffer, width, height, width * PIXFMT::pix_width);
    pixf = new pixfmt_type(rbuf);
    renderer = renbase_type(*pixf);
    renderer_solid = rensolid_type(renderer);
    
    renderer.clear(bg);
  }
  ~RenderBuffer() {
    delete pixf;
    delete [] buffer;
  }
  
  template<class COLOR>
  void init(int _width, int _height, COLOR bg) {
    delete pixf;
    delete [] buffer;
    width = _width;
    height = _height;
    buffer = new unsigned char[width * height * PIXFMT::pix_width];
    rbuf.attach(buffer, width, height, width * PIXFMT::pix_width);
    pixf = new pixfmt_type(rbuf);
    renderer = renbase_type(*pixf);
    renderer_solid = rensolid_type(renderer);
    
    renderer.clear(bg);
  }
  
  renbase_type& get_renderer() {
    return renderer;
  }
  rensolid_type& get_solid_renderer() {
    return renderer_solid;
  }
  agg::rendering_buffer& get_buffer() {
    return rbuf;
  }
  template<class COLOR>
  void set_colour(COLOR col) {
    renderer_solid.color(col);
  }
  template<class SOURCE>
  void copy_from(agg::rendering_buffer& copy_buffer) {
    agg::convert<PIXFMT, SOURCE>(&rbuf, &copy_buffer);
  }
};

class MaskBuffer : public RenderBuffer<pixfmt_type_32> {
public:
  typedef agg::scanline_u8_am<agg::alpha_mask_rgba32a> scanline_type_a;
  typedef agg::scanline_u8_am<agg::alpha_mask_rgba32gray> scanline_type_l;
  
private:
  agg::alpha_mask_rgba32a alpha_mask;
  agg::alpha_mask_rgba32gray luminance_mask;
  scanline_type_a scanline_a;
  scanline_type_l scanline_l;
  bool luminance;
  
public:
  MaskBuffer() :
  RenderBuffer<pixfmt_type_32>(),
  alpha_mask(rbuf),
  luminance_mask(rbuf),
  scanline_a(alpha_mask),
  scanline_l(luminance_mask),
  luminance(false)
  {
    
  }
  MaskBuffer(int width, int height, bool lumin) :
  RenderBuffer<pixfmt_type_32>(width, height, agg::rgba8(0, 0, 0, 0)),
  alpha_mask(rbuf),
  luminance_mask(rbuf),
  scanline_a(alpha_mask),
  scanline_l(luminance_mask),
  luminance(lumin)
  {
    
  }
  
  void init(int _width, int _height, bool lumin) {
    delete pixf;
    delete [] buffer;
    width = _width;
    height = _height;
    luminance = lumin;
    buffer = new unsigned char[width * height * 4];
    rbuf.attach(buffer, width, height, width * 4);
    pixf = new pixfmt_type_32(rbuf);
    renderer = renbase_type(*pixf);
    renderer_solid = rensolid_type(renderer);
    
    renderer.clear(agg::rgba8(0, 0, 0, 0));
  }
  
  scanline_type_a& get_masked_scanline_a() {
    return scanline_a;
  }
  scanline_type_l& get_masked_scanline_l() {
    return scanline_l;
  }
  bool use_luminance() {return luminance;}
};
