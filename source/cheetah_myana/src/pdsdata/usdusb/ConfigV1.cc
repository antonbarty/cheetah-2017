#include "pdsdata/usdusb/ConfigV1.hh"

#include <stdlib.h>

using namespace Pds::UsdUsb;

ConfigV1::ConfigV1( Count_Mode cm[],
		    Quad_Mode  qm[] ){
  for(int i = 0;i<4;i++){
    _count_mode[i] = cm[i];
    _quad_mode[i] = qm[i];
  }
}

ConfigV1::Count_Mode ConfigV1::counting_mode   (unsigned channel) const
{ 
  return Count_Mode(_count_mode[channel]);
}

ConfigV1::Quad_Mode  ConfigV1::quadrature_mode (unsigned channel) const
{
  return Quad_Mode(_quad_mode[channel]);
}

static const char* _count_mode_labels[] = { "WRAP_FULL",
					    "LIMIT",
					    "HALT",
					    "WRAP_PRESET",
					    NULL };

static const char* _quad_mode_labels[] = { "CLOCK_DIR",
					   "X1",
					   "X2",
					   "X4",
					   NULL };

const char** ConfigV1::count_mode_labels() 
{
  return _count_mode_labels;
}

const char** ConfigV1::quad_mode_labels()
{
  return _quad_mode_labels;
}
