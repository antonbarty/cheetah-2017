#ifndef ENCODERDATAV1_HH
#define ENCODERDATAV1_HH

#include <stdint.h>
#include "pdsdata/xtc/TypeId.hh"

namespace Pds
{
   namespace Encoder
   {
      class DataV1;
   }
}

class Pds::Encoder::DataV1
{
 public:
   enum { Version = 1 };

   DataV1()
      : _33mhz_timestamp( 0 ),
        _encoder_count( 0 ) {}
   DataV1( uint32_t timestamp, uint32_t count )
      : _33mhz_timestamp( timestamp ),
        _encoder_count( count ) {}

   static Pds::TypeId typeId()
      { return TypeId( TypeId::Id_EncoderData, Version ); }

   int value() const;

   uint32_t _33mhz_timestamp;
   uint32_t _encoder_count;
};

#endif
