#include "pdsdata/timepix/DataV2.hh"

#include <string.h>

using namespace Pds;
using namespace Timepix;

DataV2::DataV2(unsigned width, unsigned height,
               uint32_t timestamp, uint16_t frameCounter, uint16_t lostRows) :
  _width ((uint16_t) width),
  _height((uint16_t) height),
  _timestamp(timestamp),
  _frameCounter(frameCounter),
  _lostRows(lostRows)
{
}

// Copy constructor
DataV2::DataV2(const DataV2& f) :
  _width (f._width),
  _height(f._height),
  _timestamp(f._timestamp),
  _frameCounter(f._frameCounter),
  _lostRows(f._lostRows)
{
  memcpy(this+1,f.data(),f.data_size());
}

// Constructor for converting DataV1 to DataV2
DataV2::DataV2(const DataV1& f) :
  _width ((uint16_t) f.width()),
  _height((uint16_t) f.height()),
  _timestamp(f.timestamp()),
  _frameCounter(f.frameCounter()),
  _lostRows(f.lostRows())
{
  // convert data
  unsigned destX, destY;
  uint16_t *src = (uint16_t *)f.data();
  uint16_t *dest = (uint16_t *)(this+1);

  for (unsigned iy=0; iy < _height * 2u; iy++) {
    for (unsigned ix=0; ix < _width / 2u; ix++) {
      // map pixels from 256x1024 to 512x512
      switch (iy / 256) {
        case 0:
          destX = iy;
          destY = 511 - ix;
          break;
        case 1:
          destX = iy - 256;
          destY = 255 - ix;
          break;
        case 2:
          destX = 1023 - iy;
          destY = ix;
          break;
        case 3:
          destX = 1023 + 256 - iy;
          destY = ix + 256;
          break;
        default:
          // error
          destX = destY = 0;
          break;
      }
      dest[(destY * _width) + destX] = src[(iy * _width / 2) + ix];
    }
  }
}
