#include "agg/include/agg_pixfmt_rgb.h"
#include "agg/include/agg_pixfmt_amask_adaptor.h"
#include "agg/include/agg_alpha_mask_u8.h"
#include <Rcpp.h>
using namespace Rcpp;

enum
{
frame_width = 320,
  frame_height = 200
};

// Writing the buffer to a .PPM file, assuming it has 
// RGB-structure, one byte per color component
//--------------------------------------------------
bool write_ppm(const unsigned char* buf, 
               unsigned width, 
               unsigned height, 
               const char* file_name)
{
  FILE* fd = fopen(file_name, "wb");
  if(fd)
  {
    fprintf(fd, "P6 %d %d 255 ", width, height);
    fwrite(buf, 1, width * height * 3, fd);
    fclose(fd);
    return true;
  }
  return false;
}

// Draw a black frame around the rendering buffer, assuming it has 
// RGB-structure, one byte per color component
//--------------------------------------------------
void draw_black_frame(agg::rendering_buffer& rbuf)
{
  unsigned i;
  for(i = 0; i < rbuf.height(); ++i)
  {
    unsigned char* p = rbuf.row_ptr(i);
    *p++ = 0; *p++ = 0; *p++ = 0;
    p += (rbuf.width() - 2) * 3;
    *p++ = 0; *p++ = 0; *p++ = 0;
  }
  memset(rbuf.row_ptr(0), 0, rbuf.width() * 3);
  memset(rbuf.row_ptr(rbuf.height() - 1), 0, rbuf.width() * 3);
}

// [[Rcpp::export]]
void rcpp_hello_world() {
  // Allocate the main rendering buffer and clear it, for now "manually",
  // and create the rendering_buffer object and the pixel format renderer
  //--------------------------------
  agg::int8u* buffer = new agg::int8u[frame_width * frame_height * 3];
  memset(buffer, 255, frame_width * frame_height * 3);
  agg::rendering_buffer rbuf(buffer, 
                             frame_width, 
                             frame_height, 
                             frame_width * 3);
  agg::pixfmt_rgb24 pixf(rbuf);
  
  
  // Allocate the alpha-mask buffer, create the rendering buffer object
  // and create the alpha-mask object.
  //--------------------------------
  agg::int8u* amask_buf = new agg::int8u[frame_width * frame_height];
  agg::rendering_buffer amask_rbuf(amask_buf, 
                                   frame_width, 
                                   frame_height, 
                                   frame_width);
  agg::amask_no_clip_gray8 amask(amask_rbuf);
  
  // Create the alpha-mask adaptor attached to the alpha-mask object
  // and the pixel format renderer
  agg::pixfmt_amask_adaptor<agg::pixfmt_rgb24, 
                            agg::amask_no_clip_gray8> pixf_amask(pixf, amask);
  
  
  // Draw something in the alpha-mask buffer. 
  // In this case we fill the buffer with a simple verical gradient
  unsigned i;
  for(i = 0; i < frame_height; ++i)
  {
    unsigned val = 255 * i / frame_height;
    memset(amask_rbuf.row_ptr(i), val, frame_width);
  }
  
  
  // Draw the spectrum, write a .ppm and free memory
  //----------------------
  agg::rgba8 span[frame_width];
  
  for(i = 0; i < frame_width; ++i)
  {
    agg::rgba c(380.0 + 400.0 * i / frame_width, 0.8);
    span[i] = agg::rgba8(c);
  }
  
  for(i = 0; i < frame_height; ++i)
  {
    pixf_amask.blend_color_hspan(0, i, frame_width, span, 0);
  }
  
  write_ppm(buffer, frame_width, frame_height, "agg_test.ppm");
  
  delete [] amask_buf;
  delete [] buffer;
}
