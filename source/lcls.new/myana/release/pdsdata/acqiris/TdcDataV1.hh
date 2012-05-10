#ifndef Pds_TdcDataV1_hh
#define Pds_TdcDataV1_hh

#include <stdint.h>

namespace Pds {
  namespace Acqiris {
    class TdcDataV1 {
    public:
      enum { Version = 1 };

      class Common;
      class Channel;
      class Marker;
    public:
      TdcDataV1() {}
    public:
      enum Source { Comm, Chan1, Chan2, Chan3, Chan4, Chan5, Chan6, AuxIO };
      Source   source  () const;
    protected:
      uint32_t _value;
    };

    class TdcDataV1::Common : public TdcDataV1 {
    public:
      Common() {}
      bool     overflow() const;
      unsigned nhits   () const;
    };

    class TdcDataV1::Channel : public TdcDataV1 {
    public:
      Channel() {}
      bool     overflow() const;
      unsigned ticks   () const;
      double   time    () const;
    };

    class TdcDataV1::Marker : public TdcDataV1 {
    public:
      Marker() {}
      enum Type { AuxIOSwitch=0, EventCntSwitch=1, MemFullSwitch=2, AuxIOMarker=16 };
      Type type() const;
    };
  }
}

#endif
