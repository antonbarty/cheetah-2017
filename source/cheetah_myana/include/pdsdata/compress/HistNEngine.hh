#ifndef Pds_HistNEngine_hh
#define Pds_HistNEngine_hh

#include <cstddef>
#include <iostream>
#include <stdint.h>
#include <string.h>

namespace Pds {

  namespace Compress {

    class HistNEngine {

    public:

      struct ImageParams {
        size_t width;   // 1..
        size_t height;  // 1..
        size_t depth;   // bytes per pixel
      };

      // Status values returned due to incorrect input to operations
      enum { Success = 0,
             ErrZeroImage,         // zero pointer instead of a compressed or uncompressed image
             ErrLargeImage,        // image is is too large
             ErrSmallImage,        // image is is too small 
             ErrIllegalDepth,      // image is is too deep 
             
             // Status values returned by algorithms
             //
             ErrBadHdr,            // bad image header
             ErrBadImageData,      // inconsistent size of the image data block
             ErrBadImageBitMap,    // inconsistent size of the bitmap data block
             ErrCSMissmatch,       // checksum doesn't match the original one
             ErrBitMapMissmatch,   // the bit-map isn't consistent with the compressed image size
             ErrUnknownComprMethod // unknown compression method
      };

      HistNEngine();

      ~HistNEngine();

      int compress(const void* image, unsigned depth, size_t inDataSize,
                   void* outData, size_t& outDataSize);

      int decompress(const void* outData, size_t outDataSize, void* image, size_t image_size);

      int dump(std::ostream& str, const ImageParams& params, const void* outData, size_t outDataSize) const;

      static const char* err2str(int code);

    private:

      /* Histogram support structures have the same size irrelevant
       * of how large or small are the images to be compressed.
       */
      unsigned int* m_hist_channels;
      unsigned int* m_hist_channels_8bits;

    public:
      class Header;
      class HeaderC;
    };
  }
}

#endif
