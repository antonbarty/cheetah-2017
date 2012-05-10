//
//  Class for configuration of the Event Receiver
//
#ifndef Evr_ConfigV1_hh
#define Evr_ConfigV1_hh

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

namespace Pds {
  namespace EvrData {
    class PulseConfig;
    class OutputMap;
    class ConfigV1 {
    public:
      enum { Version=1 };
      ConfigV1 ();
      ConfigV1 (unsigned npulses,
		const PulseConfig* pulses,
		unsigned noutputs,
		const OutputMap* outputs);

      //  pulse configurations appended to this structure
      unsigned npulses() const;
      const PulseConfig& pulse(unsigned) const;

      //  output configurations appended to this structure
      unsigned noutputs() const;
      const OutputMap& output_map(unsigned) const;

      //  size including appended PulseConfig's and OutputMap's
      unsigned size() const;
    private:
      uint32_t _npulses;
      uint32_t _noutputs;
    };
  };
};
#endif
