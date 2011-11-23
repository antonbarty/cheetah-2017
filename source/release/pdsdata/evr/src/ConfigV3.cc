#include "pdsdata/evr/ConfigV3.hh"
#include "pdsdata/evr/EventCodeV3.hh"
#include "pdsdata/evr/PulseConfigV3.hh"
#include "pdsdata/evr/OutputMap.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

ConfigV3::ConfigV3(
   uint32_t neventcodes,  const EventCodeType*    eventcodes,
   uint32_t npulses,      const PulseType*  pulses,
   uint32_t noutputs,     const OutputMap*      outputs 
   ) :   
  _neventcodes(neventcodes),
  _npulses    (npulses), 
  _noutputs   (noutputs)
{
  char *next = (char*) (this + 1);

  memcpy(next, eventcodes, _neventcodes * sizeof(EventCodeType));
  next += _neventcodes * sizeof(EventCodeType);
  
  memcpy(next, pulses, _npulses * sizeof(PulseType));
  next += _npulses * sizeof(PulseType);
  
  memcpy(next, outputs, _noutputs * sizeof(OutputMap));
}

uint32_t ConfigV3::neventcodes() const
{
  return _neventcodes;
}

const ConfigV3::EventCodeType& ConfigV3::eventcode(unsigned eventcodeIndex) const
{
  const EventCodeType *eventcodes = (const EventCodeType *) (this + 1);
  return eventcodes[eventcodeIndex];  
}


uint32_t ConfigV3::npulses() const
{
  return _npulses;
}
const ConfigV3::PulseType& ConfigV3::pulse(unsigned pulse) const
{
  const PulseType *pulses = (const PulseType *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) );
    
  return pulses[pulse];
}

uint32_t ConfigV3::noutputs() const
{
  return _noutputs;
}
const OutputMap & ConfigV3::output_map(unsigned output) const
{
  const OutputMap *m = (const OutputMap *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) +
    _npulses * sizeof(PulseType) );

  return m[output];
}


unsigned ConfigV3::size() const
{
  return (sizeof(*this) + 
   _neventcodes * sizeof(EventCodeType) +
   _npulses     * sizeof(PulseType) + 
   _noutputs    * sizeof(OutputMap));
}
