#include "ragg.h"
#include "AggDevice.h"

template<class PIXFMT>
class AggDeviceRaster : public AggDevice<PIXFMT> {
public:
  AggDeviceRaster(const char* fp, int w, int h, double ps, int bg, double res) : 
  AggDevice<PIXFMT>(fp, w, h, ps, bg, res)
  {
    
  }
  // Behaviour
  bool savePage() {
    char buf[PATH_MAX+1];
    snprintf(buf, PATH_MAX, this->file.c_str(), this->pageno); buf[PATH_MAX] = '\0';
    FILE* fd = fopen(buf, "wb");
    if(fd)
    {
      fprintf(fd, "P6 %d %d 255 ", this->width, this->height);
      fwrite(this->buffer, 1,this-> width * this->height * this->bytes_per_pixel, fd);
      fclose(fd);
      return true;
    }
    return false;
  };
};

typedef AggDeviceRaster<pixfmt_type_32> AggDeviceRasterAlpha;
