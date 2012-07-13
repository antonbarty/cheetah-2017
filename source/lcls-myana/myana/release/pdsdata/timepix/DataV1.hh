// $Id: DataV1.hh,v 1.9 2012/02/16 19:37:34 caf Exp $
// Author: Chris Ford <caf@slac.stanford.edu>

//
//  Class for data of Timepix
//
//  The Timepix Quad detector includes 4 256x256 elements.
//  Software should treat it as 512x512 elements of 2 bytes each.
//
//  The appended data size is exported by the data_size() method.
//
//  Timepix::DataV1 data are NOT shuffled.
//
//  For shuffled data see Timepix::DataV2.
//
#ifndef TIMEPIXDATAV1_HH
#define TIMEPIXDATAV1_HH

#include <stdint.h>
#include "pdsdata/xtc/TypeId.hh"

namespace Pds
{
   namespace Timepix
   {
      class DataV1;
   }
}

class Pds::Timepix::DataV1
{
  public:
    enum { Version = 1 };
    enum { Height = 512 };
    enum { Width = 512 };
    enum { Depth = 14 };
    enum { DepthBytes = 2 };
    enum { RawDataBytes = Height * Width * Depth / 8 };
    enum { DecodedDataBytes = Height * Width * DepthBytes };
    enum { MaxPixelValue = 11810 };

    DataV1() :
      _timestamp(0),
      _frameCounter(0),
      _lostRows(0)
      {}

    DataV1(uint32_t timestamp, uint16_t frameCounter, uint16_t lostRows) :
      _timestamp(timestamp),
      _frameCounter(frameCounter),
      _lostRows(lostRows)
      {}

    static Pds::TypeId typeId()
      { return TypeId(TypeId::Id_TimepixData, Version); }

    unsigned width() const
      { return (Width); }

    unsigned height() const
      { return (Height); }

    unsigned depth() const
      { return (Depth); }

    // number of bytes per pixel
    unsigned depth_bytes() const
      { return (DepthBytes); }

    // size of data appended to the end of this structure
    unsigned data_size() const
      { return (DecodedDataBytes); }

    // beginning of data appended to the end of this structure
    const unsigned char* data() const
      { return reinterpret_cast<const unsigned char*>(this+1); }

    uint32_t timestamp(void) const;
    uint16_t frameCounter(void) const;
    uint16_t lostRows(void) const;

    uint32_t _timestamp;
    uint16_t _frameCounter;
    uint16_t _lostRows;
};

#endif
