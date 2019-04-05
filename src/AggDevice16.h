#include "ragg.h"
#include "AggDevice.h"
#include "util/agg_color_conv.h"

/* For htons */
#ifdef _WIN32
#include <winsock.h>
#endif

// Functor for dimming alpha if needed
struct AlphaDim {
  double alpha_mod;

  AlphaDim(double alpha_mod) : alpha_mod(alpha_mod) { }

  inline void operator () (agg::rgba16* p)
  {
      p->a *= alpha_mod;
  }
};

template<class PIXFMT>
class AggDevice16 : public AggDevice<PIXFMT, agg::rgba16> {
public:
  double alpha_mod;
  
  AggDevice16(const char* fp, int w, int h, double ps, int bg, double res, 
              double alpha_mod = 1.0) : 
    AggDevice<PIXFMT, agg::rgba16>(fp, w, h, ps, bg, res),
    alpha_mod(alpha_mod)
  {
      this->background = convertColour(this->background_int);
      this->renderer.clear(this->background);
  }
  
  void drawRaster(unsigned int *raster, int w, int h, double x, double y, 
                  double final_width, double final_height, double rot, 
                  bool interpolate) {
    agg::rendering_buffer rbuf(reinterpret_cast<unsigned char*>(raster), w, h, 
                               w * 4);
    
    unsigned char * buffer16 = new unsigned char[w * h * pixfmt_type_64::pix_width];
    agg::rendering_buffer rbuf16(buffer16, w, h, w * pixfmt_type_64::pix_width);
    
    agg::convert<pixfmt_type_64, pixfmt_r_raster>(&rbuf16, &rbuf);
    
    agg::trans_affine img_mtx;
    img_mtx *= agg::trans_affine_reflection(0);
    img_mtx *= agg::trans_affine_translation(0, h);
    img_mtx *= agg::trans_affine_scaling(final_width / double(w), 
                                         final_height / double (h));
    img_mtx *= agg::trans_affine_rotation(-rot * agg::pi / 180.0);
    img_mtx *= agg::trans_affine_translation(x, y);
    agg::trans_affine src_mtx = img_mtx;
    img_mtx.invert();
    
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator(img_mtx);
    
    typedef agg::image_accessor_clone<pixfmt_type_64> img_source_type;
    
    pixfmt_type_64 img_pixf(rbuf16);
    img_source_type img_src(img_pixf);
    agg::span_allocator<agg::rgba16> sa;
    agg::rasterizer_scanline_aa<> ras;
    ras.clip_box(this->clip_left, this->clip_top, this->clip_right, 
                 this->clip_bottom);
    agg::scanline_u8 sl;
    
    agg::path_storage rect;
    rect.remove_all();
    rect.move_to(0, 0);
    rect.line_to(0, h);
    rect.line_to(w, h);
    rect.line_to(w, 0);
    rect.close_polygon();
    agg::conv_transform<agg::path_storage> tr(rect, src_mtx);
    ras.add_path(tr);
    
    if (interpolate) {
      typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> span_gen_type;
      span_gen_type sg(img_src, interpolator);
      
      agg::render_scanlines_aa(ras, sl, this->renderer, sa, sg);
    } else {
      typedef agg::span_image_filter_rgba_nn<img_source_type, interpolator_type> span_gen_type;
      span_gen_type sg(img_src, interpolator);
      
      agg::render_scanlines_aa(ras, sl, this->renderer, sa, sg);
    }
    
    delete [] buffer16;
  }
  void to_bigend() {
    uint16_t * buf =  reinterpret_cast<uint16_t *>(this->buffer);
    for (int i = 0; i < this->width * this->height * PIXFMT::num_components; i++) {
      buf[i] = htons(buf[i]);
    }
  }
  
private:
  inline agg::rgba16 convertColour(unsigned int col) {
    agg::rgba16 new_col = agg::rgba8(R_RED(col), R_GREEN(col), R_BLUE(col), R_ALPHA(col));
    if (alpha_mod != 1.0 && new_col.a != 65535) new_col.a *= alpha_mod;
    return new_col.premultiply();
  }
};
