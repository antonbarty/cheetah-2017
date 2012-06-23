#ifndef PDS_CSPAD_CSPADCOMPRESSOR_H
#define PDS_CSPAD_CSPADCOMPRESSOR_H

//--------------------------------------------------------------------------
// File and Version Information:
//   $Id: CspadCompressor.hh,v 1.1 2012/04/04 01:18:01 tomytsai Exp $
//
// Description:
//   Class CspadCompressor. It encapsulates the compression algorithm
//   for CSPad images.
//
// Author:
//   Igor A. Gaponenko, SLAC National Accelerator Laboratory
//--------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

#include <stdint.h>
#include <sys/types.h>
#include <iosfwd>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//              ---------------------
//              -- Class Interface --
//              ---------------------

namespace Pds {

    namespace CsPad {

        class CspadCompressor {

        public:

            struct ImageParams {
                size_t width;   // 1..
                size_t height;  // 1..
                size_t depth;   // bytes per pixel
            };

            enum {

                Success = 0,

                // Status values returned due to incorrect input to operations
                //
                ErrZeroImage,         // zero pointer instead of a compressed or uncompressed image
                ErrIllegalDepth,      // illegal/unsupported image depth
                ErrIllegalWidth,      // illegal/unsupported image width
                ErrIllegalHeight,     // illegal/unsupported image height
                ErrLargeImage,        // image is is too large
                ErrSmallImage,        // image is is too small 

                // Status values returned by algorithms
                //
                ErrBadHdr,            // bad image header
                ErrBadImageData,      // inconsistent size of the image data block
                ErrBadImageBitMap,    // inconsistent size of the bitmap data block
                ErrCSMissmatch,       // checksum doesn't match the original one
                ErrBitMapMissmatch,   // the bit-map isn't consistent with the compressed image size
                ErrUnknownComprMethod // unknown compression method
            };

            CspadCompressor();

            virtual ~CspadCompressor();

            int compress(const void* image, const ImageParams& params, void*& outData, size_t& outDataSize);

            int decompress(const void* outData, size_t outDataSize, void*& image, ImageParams& params);

            int dump(std::ostream& str, const void* outData, size_t outDataSize) const;

            static const char* err2str(int code);

        private:

            /* Histogram support structures have the same size irrelevant
             * of how large or small are the images to be compressed.
             */
            unsigned int* m_hist_channels;
            unsigned int* m_hist_channels_8bits;

            /* These structures are reused accross calls to the compression method
             * in order to optimize memory management operations in case if
             * subsequent input images have the same size. Otherwise the strucures
             * will be reinitialzied to suit the storage requirements.
             */
            size_t    m_outbufsize;   // the max number of bytes available for the whole output structure
            uint8_t*  m_outbuf;       // the actual storage for the whole output structure

            size_t    m_outbmapsize;  // the size of 16-bit words in the uncompressed bitmap
            uint16_t* m_outbmap;      // temporary buffer to store uncompressed bitmap (not a part of the output structure)

            uint16_t* m_outbmapc;     // temporary storage for the compressed bitmap (not a part of the output structure);
                                      // the storage is allocated to have (exactly) twice as much space as for
                                      // uncompressed buffer due to 16-bit counters.

            /* These structures are reused accross calls to decompress
             * to optimize memory management operations in case if
             * output images have the same size. Otherwise the strucures
             * will be reinitialzied to suit the storage requirements.
             */
            size_t    m_imagesize;  // the number of 16-bit words in the image buffer
            uint16_t* m_image;      // the actual storage for uncompressed image


        };
    }
}

#endif  // PDS_CODEC_CSPADCOMPRESSOR_H
