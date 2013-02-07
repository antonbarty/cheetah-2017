//
//  Class for configuration of the Event Receiver
//
#ifndef Evr_IOConfigV1_hh
#define Evr_IOConfigV1_hh

#include "pdsdata/evr/OutputMap.hh"
#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

#pragma pack(4)

namespace Pds {
  namespace EvrData {
    class IOChannel;
    class IOConfigV1 {
    public:
      enum { Version=1 };
      IOConfigV1 ();
      IOConfigV1 (OutputMap::Conn,
		  const IOChannel*,
		  unsigned);

      OutputMap::Conn conn() const;

      unsigned nchannels() const;
      const IOChannel& channel(unsigned) const;

      unsigned size() const;
    private:
      uint16_t  _conn;
      uint16_t  _nchannels;
    };
  };
};

#pragma pack()

#endif
