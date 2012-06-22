#ifndef Evr_OutputMapV2_hh
#define Evr_OutputMapV2_hh

#include <stdint.h>

//
//  Description of the EVR output mapping.
//  Version 2 includes the EVR module in a multiple module system.
//
namespace Pds {
  namespace EvrData {
    class OutputMapV2 {
    public:
      enum Source { Pulse, DBus, Prescaler, Force_High, Force_Low };
      enum Conn { FrontPanel, UnivIO };
    public:
      OutputMapV2 ();
      OutputMapV2 ( Source, unsigned source_id, 
		    Conn  , unsigned conn_id,
		    unsigned module);
    public:
      //  source (generated pulse) of output generation
      Source   source   () const;
      unsigned source_id() const;
      //  connector for output destination
      Conn     conn     () const;
      unsigned conn_id  () const;
      unsigned module   () const;
    public:
      //  encoded source value
      unsigned map      () const;
    private:
      uint32_t _v;
    };
  };
};

#endif
