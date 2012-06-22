#ifndef ENCODERDATAV2_HH
#define ENCODERDATAV2_HH

#include <stdint.h>
#include "pdsdata/xtc/TypeId.hh"

namespace Pds
{
   namespace Encoder
   {
      class DataV2;
   }
}

class Pds::Encoder::DataV2
{
 public:
   enum { Version = 2 };

	DataV2();
	DataV2( uint32_t timestamp, uint32_t count0, uint32_t count1, uint32_t count2);
	DataV2( uint32_t timestamp, uint32_t count0);

   static Pds::TypeId typeId()
      { return TypeId( TypeId::Id_EncoderData, Version ); }

   int value(int chan) const;

   uint32_t _33mhz_timestamp;
   uint32_t _encoder_count[3];
};

#endif
