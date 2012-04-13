#include "pdsdata/princeton/FrameV1.hh"

using namespace Pds;
using namespace Princeton;

FrameV1::FrameV1( uint32_t iShotIdStart, float fReadoutTime ) :
 _iShotIdStart(iShotIdStart), _fReadoutTime(fReadoutTime)
{
}

const uint16_t* FrameV1::data() const
{
  return (uint16_t*) (this + 1);
}
