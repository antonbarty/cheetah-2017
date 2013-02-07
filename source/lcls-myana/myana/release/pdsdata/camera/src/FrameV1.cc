#include "pdsdata/camera/FrameV1.hh"

#include <string.h>

using namespace Pds;
using namespace Camera;


FrameV1::FrameV1(unsigned width, unsigned height, unsigned depth, unsigned offset) :
  _width (width),
  _height(height),
  _depth (depth),
  _offset(offset)
{
}

FrameV1::FrameV1(const FrameV1& f) :
  _width (f._width),
  _height(f._height),
  _depth (f._depth),
  _offset(f._offset)
{
  memcpy(this+1,f.data(),f.data_size());
}
