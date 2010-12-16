#ifndef Pds_SequencerEntry_hh
#define Pds_SequencerEntry_hh

#include <stdint.h>

namespace Pds {
  namespace EvrData {
    class SequencerEntry {
    public:
      SequencerEntry() {}
      SequencerEntry(unsigned eventcode,
		     unsigned delay);
    public:
      unsigned eventcode() const;
      unsigned delay    () const;
    private:
      uint32_t _value;
    };
  };
};

#endif
