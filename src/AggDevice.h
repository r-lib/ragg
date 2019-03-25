#include "ragg.h"
#include "text_renderer.h"

#include "agg_math_stroke.h"

#ifndef AGGDEV_INCLUDED
#define AGGDEV_INCLUDED

/* Base class for graphic device interface to AGG. See AggDevice.cpp for 
 * implementation details. 
 * 
 * Specific devices should subclass this and provide their own buffer and 
 * savePage() methods (at least), while the drawing methods should work 
 * regardless. The base class outputs images in ppm format which is not realy
 * a usable format. See png and tiff versions for actual usable classes.
 */
class AggDevice {
public:
  static const int bytes_per_pixel = 3;
  
  int width;
  int height;
  double clip_left;
  double clip_right;
  double clip_top;
  double clip_bottom;
  
  renbase_type renderer;
  pixfmt_type* pixf;
  agg::rendering_buffer rbuf;
  unsigned char* buffer;
  
  int pageno;
  const char* file;
  agg::rgba8 background;
  int background_int;
  double pointsize;
  
  TextRenderer t_ren;
  
  // Lifecycle methods
  AggDevice(const char* fp, int w, int h, double ps, int bg);
  ~AggDevice();
  void newPage();
  void close();
  bool savePage();
  
  // Behaviour
  void clipRect(double x0, double y0, double x1, double y1);
  double stringWidth(const char *str, const char *family, int face, 
                     double size);
  void charMetric(int c, const char *family, int face, double size,
                  double *ascent, double *descent, double *width);
  
  // Drawing Methods
  void drawCircle(double x, double y, double r, int fill, int col, double lwd, 
                  int lty, R_GE_lineend lend);
  void drawRect(double x0, double y0, double x1, double y1, int fill, int col, 
                double lwd, int lty, R_GE_lineend lend);
  void drawPolygon(int n, double *x, double *y, int fill, int col, double lwd, 
                   int lty, R_GE_lineend lend, R_GE_linejoin ljoin, 
                   double lmitre);
  void drawLine(double x1, double y1, double x2, double y2, int col, double lwd, 
                int lty, R_GE_lineend lend);
  void drawPolyline(int n, double* x, double* y, int col, double lwd, int lty,
                    R_GE_lineend lend, R_GE_linejoin ljoin, double lmitre);
  void drawPath(int npoly, int* nper, double* x, double* y, int col, int fill, 
                double lwd, int lty, R_GE_lineend lend, R_GE_linejoin ljoin, 
                double lmitre, bool evenodd);
  void drawRaster(unsigned int *raster, int w, int h, double x, double y, 
                  double final_width, double final_height, double rot, 
                  bool interpolate);
  void drawText(double x, double y, const char *str, const char *family, 
                int face, double size, double rot, double hadj, int col);
  
private:
  inline agg::rgba8 convertColour(unsigned int col) {
    return agg::rgba8(R_RED(col), R_GREEN(col), R_BLUE(col), R_ALPHA(col));
  }
  inline bool visibleColour(unsigned int col) {
    return col != NA_INTEGER && R_ALPHA(col) != 0;
  }
  inline agg::line_cap_e convertLineend(R_GE_lineend lend) {
    switch (lend) {
    case GE_ROUND_CAP:
      return agg::round_cap;
    case GE_BUTT_CAP:
      return agg::butt_cap;
    case GE_SQUARE_CAP:
      return agg::square_cap;
    }
  }
  inline agg::line_join_e convertLinejoin(R_GE_linejoin ljoin) {
    switch(ljoin) {
    case GE_ROUND_JOIN:
      return agg::round_join;
    case GE_MITRE_JOIN:
      return agg::miter_join;
    case GE_BEVEL_JOIN:
      return agg::bevel_join;
    }
  }
  template<class T>
  void makeDash(T &dash_conv, int lty, double lwd) {
    dash_conv.remove_all_dashes();
    double dash, gap;
    for(int i = 0 ; i < 8 && lty & 15 ; i += 2) {
      dash = (lty & 15) * lwd;
      lty = lty>>4;
      gap = (lty & 15) * lwd;
      lty = lty>>4;
      dash_conv.add_dash(dash, gap);
    }
  }
};

#endif
