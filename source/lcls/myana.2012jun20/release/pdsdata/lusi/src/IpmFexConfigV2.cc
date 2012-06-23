#include "pdsdata/lusi/IpmFexConfigV2.hh"

using namespace Pds::Lusi;

IpmFexConfigV2::IpmFexConfigV2(const DiodeFexConfigV2 _diode[],
			       float _xscale,
			       float _yscale) 
{
  for(int i=0; i<NCHANNELS; i++)
    diode[i] = _diode[i];
  xscale  = _xscale;
  yscale  = _yscale;
}
