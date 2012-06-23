// $Id: DataV2.hh,v 1.3 2012/02/24 01:50:41 caf Exp $
// Author: Chris Ford <caf@slac.stanford.edu>

//
//  Class for data of Timepix
//
//  The appended data size is exported by the data_size() method.
//
//  Timepix::DataV2 data are shuffled.
//
//  To shuffle Timepix::DataV1 data, use this constructor: DataV2(const DataV1&)
//
#ifndef TIMEPIXDATAV2_HH
#define TIMEPIXDATAV2_HH

#include <stdint.h>
#include "pdsdata/timepix/DataV1.hh"

namespace Pds
{
   namespace Timepix
   {
      class DataV2;
   }
}

class Pds::Timepix::DataV2
{
  public:
    enum { Version = 2 };
    enum { Depth = 14 };
    enum { MaxPixelValue = 11810 };
    DataV2() {}
    // DataV2 with unassigned contents
    DataV2(unsigned width, unsigned height,
           uint32_t timestamp, uint16_t frameCounter, uint16_t lostRows);
    // Copy constructor
    DataV2(const DataV2&);
    // Constructor for converting DataV1 to DataV2
    DataV2(const DataV1&);

    /*
     * Timepix interface
     */
    uint32_t timestamp(void) const
    {
      return (_timestamp);
    }

    uint16_t frameCounter(void) const
    {
      return (_frameCounter);
    }

    uint16_t lostRows(void) const
    {
      return (_lostRows);
    }

    inline unsigned width () const
    {
      return (unsigned) _width; 
    }

    inline unsigned height() const
    {
      return (unsigned) _height; 
    }

    inline unsigned depth () const
    {
      return Depth; 
    }

    inline unsigned depth_bytes() const
    {
      return (Depth+7)>>3;
    }

    inline unsigned data_size() const
    {
      return _width*_height*depth_bytes();
    }

    inline const unsigned char* data() const
    {
      return reinterpret_cast<const unsigned char*>(this+1);
    }

    inline const unsigned char* pixel(unsigned x,
					       unsigned y) const
    {
      return data()+(y*_width+x)*depth_bytes();
    }

  // private:
    uint16_t _width;        // pixels per row
    uint16_t _height;       // pixels per column
    uint32_t _timestamp;    // hardware timestamp
    uint16_t _frameCounter; // hardware frame counter
    uint16_t _lostRows;     // lost row count
};

#endif
