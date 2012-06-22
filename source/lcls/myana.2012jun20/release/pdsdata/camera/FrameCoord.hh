#ifndef Camera_FrameCoord_hh
#define Camera_FrameCoord_hh

#include <stdint.h>

namespace Pds {
  namespace Camera {

    class FrameCoord {
    public:
      FrameCoord() {}
      FrameCoord(unsigned short x, unsigned short y) : column(x), row(y) {}
      uint16_t column;
      uint16_t row;
    };

  };
};

#endif
