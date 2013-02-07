#include "pdsdata/fli/FrameV1.hh"

using namespace Pds;
using namespace Fli;

FrameV1::FrameV1( uint32_t iShotIdStart, float fReadoutTime ) :
 _iShotIdStart(iShotIdStart), _fReadoutTime(fReadoutTime)
{
}

const uint16_t* FrameV1::data() const
{
  return (uint16_t*) (this + 1);
}
