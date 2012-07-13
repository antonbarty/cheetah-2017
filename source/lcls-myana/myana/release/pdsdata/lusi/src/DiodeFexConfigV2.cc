#include "pdsdata/lusi/DiodeFexConfigV2.hh"

using namespace Pds::Lusi;

DiodeFexConfigV2::DiodeFexConfigV2(float _base [],
				   float _scale[])
{
  for(unsigned i=0; i<NRANGES; i++) {
    base [i] = _base [i];
    scale[i] = _scale[i];
  }
}
