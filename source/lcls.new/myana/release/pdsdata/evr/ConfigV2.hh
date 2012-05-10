//
//  Class for configuration of the Event Receiver
//
#ifndef Evr_ConfigV2_hh
#define Evr_ConfigV2_hh

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

namespace Pds {
  namespace EvrData {
    class PulseConfig;
    class OutputMap;
    class ConfigV2 {
    public:
      enum { Version=2 };
      enum RateCode { r120Hz, r60Hz, r30Hz, r10Hz, r5Hz, r1Hz, r0_5Hz, Single, NumberOfRates };
      enum BeamCode { Off, On };
      ConfigV2 ();
      ConfigV2 (BeamCode bc,
		RateCode rc,
		unsigned npulses,
		const PulseConfig* pulses,
		unsigned noutputs,
		const OutputMap* outputs);

      BeamCode beam  () const;
      RateCode rate  () const;
      unsigned opcode() const;
      static unsigned opcode(BeamCode, RateCode);

      //  pulse configurations appended to this structure
      unsigned npulses() const;
      const PulseConfig& pulse(unsigned) const;

      //  output configurations appended to this structure
      unsigned noutputs() const;
      const OutputMap& output_map(unsigned) const;

      //  size including appended PulseConfig's and OutputMap's
      unsigned size() const;
    private:
      uint32_t _opcode;
      uint32_t _npulses;
      uint32_t _noutputs;
    };
  };
};
#endif
