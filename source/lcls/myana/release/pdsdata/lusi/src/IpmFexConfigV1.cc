#include "pdsdata/lusi/IpmFexConfigV1.hh"

using namespace Pds::Lusi;

IpmFexConfigV1::IpmFexConfigV1(const DiodeFexConfigV1 _diode[],
			       float _xscale,
			       float _yscale) 
{
  for(int i=0; i<NCHANNELS; i++)
    diode[i] = _diode[i];
  xscale  = _xscale;
  yscale  = _yscale;
}
