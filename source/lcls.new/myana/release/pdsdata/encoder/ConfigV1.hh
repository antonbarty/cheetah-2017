#ifndef ENCODERCONFIGV1_HH
#define ENCODERCONFIGV1_HH

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

namespace Pds
{
   namespace Encoder
   {
      class ConfigV1;
   }
}

class Pds::Encoder::ConfigV1
{
 public:
   enum { Version = 1 };
   struct count_mode {
      enum type_t {
         WRAP_FULL,
         LIMIT,
         HALT,
         WRAP_PRESET,
         END
      };
   };
   struct quad_mode {
      enum type_t {
         CLOCK_DIR,
         X1,
         X2,
         X4,
         END
      };
   };

   ConfigV1() {}
   ConfigV1( uint32_t chan_num,
             uint32_t count_mode,
             uint32_t quadrature_mode,
             uint32_t input_num,
             uint32_t input_rising,
             uint32_t ticks_per_sec );
   ~ConfigV1() {}

   static Pds::TypeId typeId()
      { return TypeId( TypeId::Id_EncoderConfig, Version ); }

   void dump() const;

   uint32_t _chan_num;
   uint32_t _count_mode;
   uint32_t _quadrature_mode;
   uint32_t _input_num;
   uint32_t _input_rising;
   uint32_t _ticks_per_sec;

   // uint8_t       _chan_num;
   // count_mode::type_t  _count_mode;
   // quad_mode::type_t   _quadrature_mode;
   // uint8_t       _input_num;
   // bool          _input_rising;
   // uint32_t      _ticks_per_sec;
};

#endif
