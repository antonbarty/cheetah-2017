//
//  Class for data of General Standards Corporation 16AI32SSC ADC
//
#ifndef GSC16AIDATAV1_HH
#define GSC16AIDATAV1_HH

#include <stdint.h>
#include "pdsdata/xtc/TypeId.hh"

namespace Pds
{
   namespace Gsc16ai
   {
      class DataV1;
   }
}

class Pds::Gsc16ai::DataV1
{
  public:
    enum { Version = 1 };

    DataV1() {}

    static Pds::TypeId typeId()
      { return TypeId( TypeId::Id_Gsc16aiData, Version ); }

    uint16_t channelValue(int chan) const;
    uint16_t timestamp(int index) const;

    uint16_t _timestamp[3];
    //
    // The _channelData array is sized for the minimum # of channels, 1.
    // Additional channels may follow in memory.
    //
    uint16_t _channelValue[1];
};

#endif
