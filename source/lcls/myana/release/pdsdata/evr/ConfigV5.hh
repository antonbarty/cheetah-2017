//
//  Class for configuration of the Event Receiver
//
#ifndef Evr_ConfigV5_hh
#define Evr_ConfigV5_hh

#include <stdint.h>

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/evr/EventCodeV5.hh"
#include "pdsdata/evr/PulseConfigV3.hh"
#include "pdsdata/evr/OutputMap.hh"
#include "pdsdata/evr/SequencerConfigV1.hh"

#pragma pack(4)

namespace Pds
{

namespace EvrData
{

class ConfigV5
{
  /*
   * Data layout:
   *
   * ---------------
   * Data members in this class
   * --------------- 
   * Event Code configurations  (array of class EventCodeType)
   * Pulse configurations       (array of class PulseType)
   * Output Map configurations  (array of class OutputMap)
   * Sequencer configuration    (class SeqConfigType)
   */
public:
  enum { Version = 5 };

  enum { MaxPulses = 32 };
  enum { EvrOutputs = 10 };

  typedef EventCodeV5       EventCodeType;  
  typedef PulseConfigV3     PulseType;  
  typedef OutputMap         OutputMapType;  
  typedef SequencerConfigV1 SeqConfigType;

  ConfigV5(
    uint32_t neventcodes, const EventCodeType* eventcodes,
    uint32_t npulses,     const PulseType*     pulses,
    uint32_t noutputs,    const OutputMap*     outputs,
    const SeqConfigType& seq_config);    

  //  event codes appended to this structure   
  uint32_t                neventcodes ()          const;
  const  EventCodeType&   eventcode   (unsigned)  const;

  //  pulse configurations appended to this structure
  uint32_t          npulses     ()          const;
  const PulseType&  pulse       (unsigned)  const;

  //  output configurations appended to this structure
  uint32_t              noutputs    ()          const;
  const OutputMap&      output_map  (unsigned)  const;

  const SeqConfigType&  seq_config() const;

  //  size including appended EventCode's, PulseType's and OutputMap's
  unsigned        size() const;
  
private:
  uint32_t _neventcodes;
  uint32_t _npulses;
  uint32_t _noutputs;
};

} // namespace EvrData
} // namespace Pds

#pragma pack()

#endif
