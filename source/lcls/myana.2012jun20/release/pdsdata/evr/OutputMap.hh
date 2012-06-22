#ifndef Evr_OutputMap_hh
#define Evr_OutputMap_hh

#include <stdint.h>

namespace Pds {
  namespace EvrData {
    class OutputMap {
    public:
      enum Source { Pulse, DBus, Prescaler, Force_High, Force_Low };
      enum Conn { FrontPanel, UnivIO };
    public:
      OutputMap ();
      OutputMap ( Source, unsigned source_id, 
		  Conn  , unsigned conn_id );
    public:
      //  source (generated pulse) of output generation
      Source   source   () const;
      unsigned source_id() const;
      //  connector for output destination
      Conn     conn     () const;
      unsigned conn_id  () const;
    public:
      //  encoded source value
      unsigned map      () const;
    private:
      uint32_t _v;
    };
  };
};

#endif
