#include "pdsdata/lusi/DiodeFexConfigV1.hh"

using namespace Pds::Lusi;

DiodeFexConfigV1::DiodeFexConfigV1(float _base [],
				   float _scale[])
{
  for(unsigned i=0; i<NRANGES; i++) {
    base [i] = _base [i];
    scale[i] = _scale[i];
  }
}
