#include "pdsdata/evr/ConfigV5.hh"
#include "pdsdata/evr/EventCodeV5.hh"
#include "pdsdata/evr/PulseConfigV3.hh"
#include "pdsdata/evr/OutputMap.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

ConfigV5::ConfigV5(
   uint32_t neventcodes,  const EventCodeType*  eventcodes,
   uint32_t npulses,      const PulseType*      pulses,
   uint32_t noutputs,     const OutputMap*      outputs,
   const SeqConfigType& seq_config ) :   
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
  next += _noutputs * sizeof(OutputMap);

  memcpy(next, &seq_config, seq_config.size());
}

uint32_t ConfigV5::neventcodes() const
{
  return _neventcodes;
}

const ConfigV5::EventCodeType& ConfigV5::eventcode(unsigned eventcodeIndex) const
{
  const EventCodeType *eventcodes = (const EventCodeType *) (this + 1);
  return eventcodes[eventcodeIndex];  
}


uint32_t ConfigV5::npulses() const
{
  return _npulses;
}
const ConfigV5::PulseType& ConfigV5::pulse(unsigned pulse) const
{
  const PulseType *pulses = (const PulseType *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) );
    
  return pulses[pulse];
}

uint32_t ConfigV5::noutputs() const
{
  return _noutputs;
}
const OutputMap & ConfigV5::output_map(unsigned output) const
{
  const OutputMap *m = (const OutputMap *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) +
    _npulses * sizeof(PulseType) );

  return m[output];
}

const SequencerConfigV1 & ConfigV5::seq_config() const
{
  return *reinterpret_cast<const SeqConfigType*>(&output_map(_noutputs));
}

unsigned ConfigV5::size() const
{
  return (sizeof(*this) + 
          _neventcodes * sizeof(EventCodeType) +
          _npulses     * sizeof(PulseType) + 
          _noutputs    * sizeof(OutputMap) +
          seq_config().size());
}
