#include "pdsdata/fccd/FccdConfigV1.hh"
#include "pdsdata/camera/FrameCoord.hh"

#include <string.h>

using namespace Pds;
using namespace FCCD;

FccdConfigV1::FccdConfigV1 () {}

FccdConfigV1::FccdConfigV1 (
  uint16_t u16OutputMode
  ) :
  _u16OutputMode          (u16OutputMode)
{}
