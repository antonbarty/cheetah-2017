//
//  Class for configuration of the Event Sequencer
//
#ifndef Evr_SequencerConfigV1_hh
#define Evr_SequencerConfigV1_hh

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

namespace Pds {
  namespace EvrData {
    class SequencerEntry;
    class SequencerConfigV1 {
    public:
      enum { Version=1 };
      enum Source { r120Hz, r60Hz, r30Hz, r10Hz, r5Hz, r1Hz, r0_5Hz, Disable };
      SequencerConfigV1 () {}
      SequencerConfigV1 (Source   sync_source,
			 Source   beam_source,
			 unsigned length,
			 unsigned cycles,
			 const SequencerEntry* entries);
      SequencerConfigV1 (const SequencerConfigV1&);

      Source   sync_source() const;
      Source   beam_source() const;
      unsigned length     () const;
      unsigned cycles     () const;
      const SequencerEntry& entry(unsigned) const;

      unsigned size() const;
    private:
      uint32_t _source;
      uint32_t _length;
      uint32_t _cycles;
    };
  };
};
#endif
