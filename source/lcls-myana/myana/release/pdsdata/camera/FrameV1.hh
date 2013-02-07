//
//  Class for rectangular frame data
//
#ifndef Pds_FrameV1_hh
#define Pds_FrameV1_hh

#include <stdint.h>

namespace Pds {

  namespace Camera {

    class FrameV1 {
    public:
      enum {Version=1};
      FrameV1() {}
      //  FrameV1 with unassigned contents
      FrameV1(unsigned width, unsigned height, unsigned depth, unsigned offset);
      //  Copy constructor
      FrameV1(const FrameV1&);
    public:
      //  number of pixels in a row
      unsigned width () const;
      //  number of pixels in a column
      unsigned height() const;
      //  number of bits per pixel
      unsigned depth () const;
      //  fixed offset/pedestal value of pixel data
      unsigned offset() const;

      // number of bytes per pixel
      unsigned depth_bytes() const;
      // size of pixel data appended to the end of this structure
      unsigned data_size  () const;

      //  beginning of pixel data
      const unsigned char* data() const;
      //  location of individual pixel datum
      const unsigned char* pixel(unsigned x,unsigned y) const;
    private:
      uint32_t _width;  // pixels per row
      uint32_t _height; // pixels per column
      uint32_t _depth;  // bits per pixel
      uint32_t _offset; // fixed offset/pedestal value of pixel data
    };

    inline unsigned FrameV1::width () const
    {
      return _width; 
    }

    inline unsigned FrameV1::height() const
    {
      return _height; 
    }

    inline unsigned FrameV1::depth () const
    {
      return _depth; 
    }

    inline unsigned FrameV1::offset() const
    {
      return _offset; 
    }

    inline unsigned FrameV1::depth_bytes() const
    {
      return (_depth+7)>>3;
    }

    inline unsigned FrameV1::data_size() const
    {
      return _width*_height*depth_bytes();
    }

    inline const unsigned char* FrameV1::data() const
    {
      return reinterpret_cast<const unsigned char*>(this+1);
    }

    inline const unsigned char* FrameV1::pixel(unsigned x,
					       unsigned y) const
    {
      return data()+(y*_width+x)*depth_bytes();
    }

  };
};
#endif
