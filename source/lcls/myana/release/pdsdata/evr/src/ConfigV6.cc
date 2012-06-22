#include "pdsdata/evr/ConfigV6.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

ConfigV6::ConfigV6(
   uint32_t neventcodes,  const EventCodeType*  eventcodes,
   uint32_t npulses,      const PulseType*      pulses,
   uint32_t noutputs,     const OutputMapType*  outputs,
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
  
  memcpy(next, outputs, _noutputs * sizeof(OutputMapType));
  next += _noutputs * sizeof(OutputMapType);

  memcpy(next, &seq_config, seq_config.size());
}

uint32_t ConfigV6::neventcodes() const
{
  return _neventcodes;
}

const ConfigV6::EventCodeType& ConfigV6::eventcode(unsigned eventcodeIndex) const
{
  const EventCodeType *eventcodes = (const EventCodeType *) (this + 1);
  return eventcodes[eventcodeIndex];  
}


uint32_t ConfigV6::npulses() const
{
  return _npulses;
}
const ConfigV6::PulseType& ConfigV6::pulse(unsigned pulse) const
{
  const PulseType *pulses = (const PulseType *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) );
    
  return pulses[pulse];
}

uint32_t ConfigV6::noutputs() const
{
  return _noutputs;
}
const ConfigV6::OutputMapType & ConfigV6::output_map(unsigned output) const
{
  const OutputMapType *m = (const OutputMapType *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) +
    _npulses * sizeof(PulseType) );

  return m[output];
}

const SequencerConfigV1 & ConfigV6::seq_config() const
{
  return *reinterpret_cast<const SeqConfigType*>(&output_map(_noutputs));
}

unsigned ConfigV6::size() const
{
  return (sizeof(*this) + 
          _neventcodes * sizeof(EventCodeType) +
          _npulses     * sizeof(PulseType) + 
          _noutputs    * sizeof(OutputMapType) +
          seq_config().size());
}
