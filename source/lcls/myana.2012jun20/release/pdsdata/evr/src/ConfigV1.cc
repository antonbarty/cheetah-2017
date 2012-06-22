#include "pdsdata/evr/ConfigV1.hh"
#include "pdsdata/evr/PulseConfig.hh"
#include "pdsdata/evr/OutputMap.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

ConfigV1::ConfigV1 () {}

ConfigV1::ConfigV1 (unsigned npulses,
		    const PulseConfig* pulses,
		    unsigned noutputs,
		    const OutputMap* outputs) :
  _npulses (0),
  _noutputs(0)
{
  char* next = reinterpret_cast<char*>(this+1);
  memcpy(next, pulses, npulses*sizeof(PulseConfig));
  _npulses=npulses;
  next += npulses*sizeof(PulseConfig);
  memcpy(next, outputs, noutputs*sizeof(OutputMap));
  _noutputs=noutputs;
}


unsigned ConfigV1::npulses() const { return _npulses; }
const PulseConfig& ConfigV1::pulse(unsigned pulse) const
{
  const PulseConfig* p = reinterpret_cast<const PulseConfig*>(this+1);
  return p[pulse];
}

unsigned ConfigV1::noutputs() const { return _noutputs; }
const OutputMap& ConfigV1::output_map(unsigned output) const
{
  const OutputMap* m = reinterpret_cast<const OutputMap*>(&pulse(_npulses));
  return m[output];
}


unsigned ConfigV1::size() const
{ 
  return (sizeof(*this) + 
	  _npulses*sizeof(PulseConfig) + 
	  _noutputs*sizeof(OutputMap));
}

