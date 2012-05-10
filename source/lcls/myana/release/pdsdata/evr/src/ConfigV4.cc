#include "pdsdata/evr/ConfigV4.hh"
#include "pdsdata/evr/EventCodeV4.hh"
#include "pdsdata/evr/PulseConfigV3.hh"
#include "pdsdata/evr/OutputMap.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

ConfigV4::ConfigV4(
   uint32_t neventcodes,  const EventCodeType*  eventcodes,
   uint32_t npulses,      const PulseType*      pulses,
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

uint32_t ConfigV4::neventcodes() const
{
  return _neventcodes;
}

const ConfigV4::EventCodeType& ConfigV4::eventcode(unsigned eventcodeIndex) const
{
  const EventCodeType *eventcodes = (const EventCodeType *) (this + 1);
  return eventcodes[eventcodeIndex];  
}


uint32_t ConfigV4::npulses() const
{
  return _npulses;
}
const ConfigV4::PulseType& ConfigV4::pulse(unsigned pulse) const
{
  const PulseType *pulses = (const PulseType *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) );
    
  return pulses[pulse];
}

uint32_t ConfigV4::noutputs() const
{
  return _noutputs;
}
const OutputMap & ConfigV4::output_map(unsigned output) const
{
  const OutputMap *m = (const OutputMap *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) +
    _npulses * sizeof(PulseType) );

  return m[output];
}


unsigned ConfigV4::size() const
{
  return (sizeof(*this) + 
   _neventcodes * sizeof(EventCodeType) +
   _npulses     * sizeof(PulseType) + 
   _noutputs    * sizeof(OutputMap));
}
