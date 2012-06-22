//-----------------------
// This Class's Header --
//-----------------------

#include "pdsdata/cspad/CspadCompressor.hh"

//-----------------
// C/C++ Headers --
//-----------------

#include <string.h>

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

//-------------------------------
// Collaborating Class Headers --
//-------------------------------


//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------


namespace {
    string format_addr( size_t addr )
    {
        ostringstream s;
        s << setw(6) << setfill('0') << addr;
        return s.str();
    }
}


//              ----------------------------------------
//              -- Public Function Member Definitions --
//              ----------------------------------------

namespace Pds {
namespace CsPad {

CspadCompressor::CspadCompressor() :
    m_hist_channels      (new unsigned int[0x10000]),
    m_hist_channels_8bits(new unsigned int[0x100]),

    m_outbufsize (0),
    m_outbuf     (0),
    m_outbmapsize(0),
    m_outbmap    (0),
    m_outbmapc   (0),

    m_imagesize  (0),
    m_image      (0)
{}

CspadCompressor::~CspadCompressor()
{
    delete [] m_hist_channels;
    delete [] m_hist_channels_8bits;
    delete [] m_outbuf;
    delete [] m_outbmap;
    delete [] m_outbmapc;
    delete [] m_image;
}

int
CspadCompressor::compress (
    const void*        image,
    const ImageParams& params,
    void*&             outData,
    size_t&            outDataSize )
{
    /* Evaluate input parameters and refuse to proceed if any obvious
     * problems were found.
     */
    if( 0 == image )         return ErrZeroImage;     // zero pointer instead of an input image
    if( params.depth  != 2 ) return ErrIllegalDepth;  // only allowing 16-bit values on the input
    if( params.width  == 0 ) return ErrIllegalWidth;  //
    if( params.height == 0 ) return ErrIllegalHeight; //

    if( params.width * params.height > 512*1024*1024 ) return ErrLargeImage;  // input image is too large

    /* The temporary variables for the input buffer. The variables
     * are expressed in terms of 16-bit unsigned integers.
     */
    const size_t    inbufsize = params.width * params.height;
    const uint16_t* inbuf     = (uint16_t*)image;
 
    /* Allocate memory for output buffers.
     *
     * If this is the very first call to the functon then allocate
     * space w/o checking. Otherwise check if the image parameters are different
     * from the previous call to the function and reallocate new buffers
     * according to the request.
     */
    const size_t prev_outbufsize = m_outbufsize;

    m_outbufsize =
        sizeof(uint32_t)     +             // - storage for compression flag(s): data, bitmap, etc.
        sizeof(uint32_t)     +             // - storage for the data checksum (either compressed or not)
        sizeof(uint32_t) * 3 +             // - storage for image params (width, height, depth)
        sizeof(uint32_t)     +             // - storage for a size of uncompressed data (shorts)
        sizeof(uint32_t)     +             // - storage for the data buffer length (bytes)
        sizeof(uint16_t)     * inbufsize;  // - storage for the data buffer:
                                           //
                                           //   - in the worst case scenario when nothing gets compressed
                                           //     it will be just original data
                                           //
                                           //   - for the compressed data we expect the compressed data
                                           //     plus overheads  not to exceed the uncompressed data size.

    bool reallocate_buffers = false;
    if( 0 == prev_outbufsize )
        reallocate_buffers = true;
    else {
        if( prev_outbufsize != m_outbufsize ) {
            delete [] m_outbuf;
            delete [] m_outbmap;
            delete [] m_outbmapc;
            reallocate_buffers = true;
        }
    }
    if( reallocate_buffers ) {
        m_outbuf      = new uint8_t [m_outbufsize];

        m_outbmapsize = inbufsize/ 16 + 1;
        m_outbmap     = new uint16_t [m_outbmapsize];  // have enough room to record copression state
                                                       // of each input word

        m_outbmapc    = new uint16_t [m_outbmapsize * 2];  // worst case scenario - nothing gets compressed
                                                           // so we just double the amount of data by adding
                                                           // counters.
    }

    /* Rotating checksum calculated on the original uncompressed data
     */
    uint32_t incs = 0;

    /* Evaluate the input buffer to see if it makes a sense to compress it.
     * If so then we'll also calculate a common base for the pixel values
     * as well as the original checksum on the image.
     * Otherwise just copy it "as is" into the output structure.
     */
    {
        memset( m_hist_channels,       0, sizeof(unsigned int)*0x10000 );
        memset( m_hist_channels_8bits, 0, sizeof(unsigned int)*0x100   );

        const uint16_t* end = inbuf + inbufsize;
        for( const uint16_t* ptr = inbuf; ptr < end; ++ptr ) {
            m_hist_channels[*ptr]++;
            m_hist_channels_8bits[(*ptr)>>8]++;
            incs += *ptr;
        }
    }

    // At the first pass we're going to see if there are at least 1/2
    // of elements in any two adjusent bins of the 8-bits/bin histogram.
    // If not then it won't make any sense to proceed with the compression
    // because the compression rate would be to low due to added overhead
    // of the bit-map index.
    //
    // This step (once we establish a case for the compression) may also
    // seegnificantly speedup the next step of locating a cluster of elements
    // to be compressed.
    //
    const unsigned int* start_8bits     = m_hist_channels_8bits + 0x100;
    unsigned int        uMax2BlockCount = 0;
    for( unsigned int* ptr = m_hist_channels_8bits;
                       ptr < m_hist_channels_8bits + 0x100 - 1;
                     ++ptr ) {
        unsigned int u2BlockCount = *(ptr+1) + *ptr;
        if (u2BlockCount >= uMax2BlockCount)
        {
          uMax2BlockCount = u2BlockCount;
          start_8bits = ptr;
        }
    }
    
    if( uMax2BlockCount < inbufsize / 2 ) {

        /* NO COMPRESSION: just fill in the header and copy the contents of
         *                 the input buffer into the output one and return
         *                 from the method.
         */
        uint8_t* ptr = m_outbuf;

        *(uint32_t*)ptr = 0x0;                          ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = incs;                         ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = params.width;                 ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = params.height;                ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = params.depth;                 ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = inbufsize * sizeof(uint16_t); ptr += sizeof(uint32_t);

        memcpy((void*)ptr, image, inbufsize * sizeof(uint16_t));
        ptr += inbufsize * sizeof(uint16_t);

        outData     = (void*)m_outbuf;
        outDataSize = ptr - m_outbuf;

        return Success;
    }

    // The second pass has two stages. At the first one a sweep accross
    // the first 256 elements is made in order to calculate the sum of counters.
    // At the second stage the alforithm will be sliding the window through
    // the remaining 256 elements to determine the window (of 256 bins) where
    // the maximum number of counts is found. For optimization purposes, the
    // the second state algorithm will use the sum from each previous iteration.
    //
    //unsigned int count_8bits = 0;
    //unsigned int* start = m_hist_channels + ( start_8bits - m_hist_channels_8bits ) * 0x100;
    //for( unsigned int* ptr = start;
    //                   ptr < start + 0x100;
    //                 ++ptr ) count_8bits += *ptr;
    
    unsigned int count_8bits = *start_8bits;    
    unsigned int* start = m_hist_channels + ( start_8bits - m_hist_channels_8bits ) * 0x100;
    unsigned int* start4compression = start;
    unsigned int new_count_8bits = count_8bits;
    for( unsigned int* ptr = start;
                       ptr < start + 0x100;
                     ++ptr ) {
        new_count_8bits += *(ptr+0x100) - *(ptr);
        if( new_count_8bits > count_8bits ) {
            count_8bits = new_count_8bits;
            start4compression = ptr + 1;                        
        }
    }

    /* Base to offset compressed numbers
     */
    const uint16_t base = start4compression - m_hist_channels;

    /* Phase I: compress the elements, build the bit-map to indicate
     *          which elements have been compressed.
     */
    uint16_t bit  = 0x1;  // current bit in the bitmap
    uint16_t bmap = 0x0;  // current bitmap word

    /* Initialize the output buffer's header.
     */
    uint8_t* outptr = m_outbuf;

    *(uint32_t*)outptr = 0;              outptr += sizeof(uint32_t);  // skip compression flag
    *(uint32_t*)outptr = 0;              outptr += sizeof(uint32_t);  // skip checksum
    *(uint32_t*)outptr = params.width;   outptr += sizeof(uint32_t);
    *(uint32_t*)outptr = params.height;  outptr += sizeof(uint32_t);
    *(uint32_t*)outptr = params.depth;   outptr += sizeof(uint32_t);
    *(uint32_t*)outptr = 0;              outptr += sizeof(uint32_t);  // skip the final data size
    *(uint16_t*)outptr = 0;              outptr += sizeof(uint16_t);  // skip the base
    *(uint32_t*)outptr = 0;              outptr += sizeof(uint32_t);  // skip the compressed image size

    const uint8_t*  outptr_after_header = outptr;
    
    int iCount8Bit = 0, iCount16Bita = 0, iCount16Bitb = 0;
    /* Prepare a pointer onto the uncompressed bitmap
     */
    uint16_t* outbmapptr = m_outbmap;
    {
        const uint16_t* end = inbuf + inbufsize;
        for( const uint16_t* ptr = inbuf; ptr < end; ++ptr ) {

            if( *ptr >= base ) {
                const uint16_t v_offset = *ptr - base;
                if( v_offset < 0x100 ) {
                    *(outptr++) = (uint8_t)v_offset;
                    bmap |= bit;
                    ++iCount8Bit;
                } else {
                    *(uint16_t*)outptr = *ptr;  // store 'as is' (16-bit word)
                    outptr += sizeof(uint16_t);
                    ++iCount16Bita;
                }
            } else {
                *(uint16_t*)outptr = *ptr;  // store 'as is' (16-bit word)
                outptr += sizeof(uint16_t);
                ++iCount16Bitb;
            }
            bit <<= 1;
            if( 0x0 == bit ) {
                *(outbmapptr++) = bmap;
                bit  = 0x1;
                bmap = 0x0;
            }
        }
        if( 0x1 != bit ) *(outbmapptr++) = bmap;    // Make sure the last bit map word is stored
                                                    // if it was't complete within the loop.
    }
    
    /* Phase II: Try to compress the bitmap using runlength compression
     *           algorithm (counting words with fully populated bits).
     *
     * ATTENTION: If the compressed bitmap will have the bigger size than
     *            the uncompressed one then the uncompressed bitmap will
     *            be transfered into the output buffer. The compression flag
     *            be also updated according to store this.
     */
    uint16_t* outbmapcomprptr = m_outbmapc;
    uint16_t  counter = 0;

    for( uint16_t* ptr = m_outbmap; ptr < outbmapptr; ++ptr ) {

        const uint16_t bmap = *ptr;

        if( 0xFFFF == bmap ) {
            if( 0xFFFF == counter ) {

                /* Flush to the storage as we've reached the representation range
                 * limit (16-bits) for the counter.
                 */
                *(outbmapcomprptr++) = counter;
                *(outbmapcomprptr++) = bmap;
                counter = 0;
            } else {
                ++counter;
            }
        } else {
            if( 0 != counter ) {

                // Flush the previously accumulated counter to the storage because
                // the previous sequence has just ended.
                //
                *(outbmapcomprptr++) = counter;
                *(outbmapcomprptr++) = 0xFFFF;
                counter = 0;
             }
            *(outbmapcomprptr++) = 1;
            *(outbmapcomprptr++) = bmap;
        }
    }
    if( 0 != counter ) {

        /* Flush the previously accumulated counter to the storage because
         * the previous sequence has just ended.
         */
        *(outbmapcomprptr++) = counter;
        *(outbmapcomprptr++) = 0xFFFF;
        counter = 0;
    }
    const size_t outbmapcsize = outbmapcomprptr - m_outbmapc;    
    
    /* COMPRESSION: fill in the header and compressed data (image itself
     * and (compressed or not) bitmap.
     */
    {
        uint8_t* ptr = m_outbuf;

        const uint32_t  compression_flag = outbmapcsize < m_outbmapsize ? 3 : 1;
        const size_t    bitmapsize       = outbmapcsize < m_outbmapsize ? outbmapcsize : m_outbmapsize;
        const uint16_t* bitmapptr        = outbmapcsize < m_outbmapsize ? m_outbmapc   : m_outbmap;

        const size_t data_size               =
            sizeof(uint16_t)                 +  // storage for the base
            sizeof(uint32_t)                 +  // storage for a size of the compressed image (bytes)
            ( outptr - outptr_after_header ) +  // storage for the compressed image itself
            sizeof(uint32_t)                 +  // storage for the size of the (compressed or not) bitmap itself (shorts)
            bitmapsize * sizeof(uint16_t);      // storage for the (compressed or not) bitmap itself

        *(uint32_t*)ptr = compression_flag;             ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = incs;                         ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = params.width;                 ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = params.height;                ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = params.depth;                 ptr += sizeof(uint32_t);
        *(uint32_t*)ptr = data_size;                    ptr += sizeof(uint32_t);
        *(uint16_t*)ptr = base;                         ptr += sizeof(uint16_t);
        *(uint32_t*)ptr = outptr - outptr_after_header;

        /* Now set the pointer beyond the last byte of the compressed image
         * and add the bitmap size and the bitmap itself.
         */
        ptr = outptr;
        *(uint32_t*)ptr = bitmapsize; ptr += sizeof(uint32_t);
        
        memcpy((void*)ptr, bitmapptr, bitmapsize * sizeof(uint16_t));
        ptr +=  bitmapsize * sizeof(uint16_t);

        outData     =       (void*)m_outbuf;
        outDataSize = ptr -        m_outbuf;
    }
        
    return Success;
}

int
CspadCompressor::decompress (
    const void*  outData,
    size_t       outDataSize,
    void*&       image,
    ImageParams& params )
{
    /* Evaluate input parameters and refuse to proceed if any obvious
     * problems were found.
     */
    if( 0 == outData ) return ErrZeroImage;  // zero pointer instead of a compressed image

    const size_t hdr_size_bytes =
        sizeof(uint32_t)        +  // compression flags
        sizeof(uint32_t)        +  // original image checksum
        sizeof(uint32_t)        +  // width
        sizeof(uint32_t)        +  // height
        sizeof(uint32_t)        +  // depth
        sizeof(uint32_t)        ;  // compressed data block size

    if( outDataSize < hdr_size_bytes )

        return ErrSmallImage;  // compressed image is too small

    uint8_t*       hdr_ptr               = (uint8_t*)outData;
    const uint32_t hdr_compression_flag  = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);
    const uint32_t hdr_original_checksum = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);
    params.width                         = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);
    params.height                        = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);
    params.depth                         = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);

    if( 2 != params.depth ) return ErrIllegalDepth;  // unsupported image depth

    const size_t hdr_compressed_size_bytes = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);

    if( hdr_size_bytes + hdr_compressed_size_bytes != outDataSize )

        return ErrBadHdr;  // inconsistent size of the compressed image
                           // and its payload.

    /* Allocate memory for output image.
     *
     * If this is the very first call to the functon then allocate
     * space w/o checking. Otherwise check if the image parameters are different
     * from the previous call to the function and reallocate new buffers
     * according to the request.
     */
    const size_t prev_imagesize = m_imagesize;

    m_imagesize = params.width * params.height;

    bool reallocate_buffers = false;
    if( 0 == prev_imagesize ) reallocate_buffers = true;
    else {
        if( prev_imagesize != m_imagesize ) {
            delete [] m_image;
            reallocate_buffers = true;
        }
    }
    if( reallocate_buffers )
        m_image = new uint16_t [m_imagesize];

    if( 0 == hdr_compression_flag ) {

        /* NO COMPRESSION: of the original image. Just copy the image over and
         * recalculate the checksum to make sure nothing has been lost.
         */
        if( m_imagesize * sizeof(uint16_t) != hdr_compressed_size_bytes )

            return ErrBadImageData;  // inconsistent size of the uncompressed
                                     // data block.

        uint32_t  cs        = 0;
        uint16_t* out_ptr   = m_image;
        uint16_t* ptr_begin = (uint16_t*)hdr_ptr;
        uint16_t* ptr_end   = ptr_begin + m_imagesize;

        for( uint16_t* ptr = ptr_begin; ptr != ptr_end; ++ptr ) {
            *(out_ptr++) = *ptr;
            cs += *ptr;
        }
        if( hdr_original_checksum != cs )

            return ErrCSMissmatch;  // checksum doesn't match the original one

        image = (void*)m_image;

        return Success;
    }

    /* COMPRESSED IMAGE: uncompressed using the attached bitmap.
     * If the last one is also compressed then uncompress it on the fly.
     */
    const uint16_t base            = *(uint16_t*)hdr_ptr; hdr_ptr += sizeof(uint16_t);
    const size_t   data_size_bytes = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);

    if( hdr_compressed_size_bytes <
        sizeof(uint16_t) +
        sizeof(uint32_t) + data_size_bytes )

        return ErrBadImageData;  // inconsistent size of the compressed
                                 // data block.

    uint8_t*     data_ptr           = hdr_ptr;
    uint8_t*     bitmap_size_ptr    = data_ptr + data_size_bytes;
    const size_t bitmap_size_shorts = *(uint32_t*)bitmap_size_ptr; bitmap_size_ptr += sizeof(uint32_t);
    uint16_t*    bitmap_ptr         = (uint16_t*)bitmap_size_ptr;

    if( hdr_compressed_size_bytes !=
        sizeof(uint16_t) +                      // base
        sizeof(uint32_t) +                      // compressed image itself
        data_size_bytes  +       
        sizeof(uint32_t) +
        sizeof(uint16_t) * bitmap_size_shorts )

            return ErrBadImageBitMap;  // inconsistent size of the bitmap
                                       // data block.

    uint32_t  cs      = 0;
    uint16_t* outptr  = m_image;
    uint8_t*  ptr     = data_ptr;
    uint8_t*  ptr_end = data_ptr + data_size_bytes;
    uint16_t  v       = 0;

    if( 3 == hdr_compression_flag ) {

        /* COMPRESSED BIT-MAP: make sure it's consistent with the compressed
         * image.
         */

        bool done = false;

        for( uint16_t* mptr = bitmap_ptr; !done; ) {

            const uint16_t counter = *(mptr++);
            const uint16_t bmap    = *(mptr++);

            /* TODO: consider optimizing the code for simple bitmaps (0x0 or 0xFFFF)
             */
            for( uint16_t i = 0; i < counter; ++i ) {

                uint16_t bit = 0x1;

                do {
                    if( bmap & bit ) v = base + *(ptr++);
                    else {
                        v = *((uint16_t*)ptr);
                        ptr += sizeof(uint16_t);
                    }
                    *(outptr++) = v;
                    cs += v;

                    if( ptr >= ptr_end ) {

                        if( outptr != m_image + m_imagesize )

                            return ErrBitMapMissmatch;  // the compressed bit-map isn't consistent with the compressed
                                                        // image size

                        done = true;
                        break;
                    }
                    bit <<= 1;

                } while( bit );

                if( done ) break;
            }
        }

    } else if( 1 == hdr_compression_flag ) {

        /* NON-COMPRESSED BITMAP:
         */

        bool done = false;

        for( uint16_t* mptr = bitmap_ptr; !done; ++mptr ) {

            const uint16_t bmap = *mptr;

            /* TODO: consider optimizing the code for simple bitmaps (0x0 or 0xFFFF)
             */
            uint16_t bit = 0x1;

            do {
                if( bmap & bit ) v = base + *(ptr++);
                else {
                    v = *((uint16_t*)ptr);
                    ptr += sizeof(uint16_t);
                }
                *(outptr++) = v;
                cs += v;

                if( ptr >= ptr_end ) {

                    if( outptr != m_image + m_imagesize )

                        return ErrBitMapMissmatch;  // the uncompressed bit-map isn't consistent with
                                                    // the compressed image size

                    done = true;
                    break;
                }
                bit <<= 1;

            } while( bit );

            if( done ) break;
        }

    } else {
        return ErrUnknownComprMethod;  // unsupported compresion algorithm
    }
    if( hdr_original_checksum != cs )

        return ErrCSMissmatch;  // checksum doesn't match

    image = (void*)m_image;

    return Success;
}

int
CspadCompressor::dump(std::ostream& str,
                      const void* outData,
                      size_t outDataSize) const
{
    /* Evaluate input parameters and refuse to proceed if any obvious
     * problems were found.
     */
    if( 0 == outData ) return ErrZeroImage;  // zero pointer instead of a compressed image

    const size_t hdr_size_bytes =
        sizeof(uint32_t)        +  // compression flags
        sizeof(uint32_t)        +  // original image checksum
        sizeof(uint32_t)        +  // width
        sizeof(uint32_t)        +  // height
        sizeof(uint32_t)        +  // depth
        sizeof(uint32_t)        ;  // compressed data block size

    if( outDataSize < hdr_size_bytes )

        return ErrSmallImage;  // compressed image is too small

    ImageParams params;

    uint8_t*       hdr_ptr               = (uint8_t*)outData;
    const uint32_t hdr_compression_flag  = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);
    const uint32_t hdr_original_checksum = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);
    params.width                         = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);
    params.height                        = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);
    params.depth                         = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);

    if( 2 != params.depth ) return ErrIllegalDepth;  // unsupported image depth

    const size_t hdr_compressed_size_bytes = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);

    if( hdr_size_bytes + hdr_compressed_size_bytes != outDataSize )

        return ErrBadHdr;  // inconsistent size of the compressed image
                           // and its payload.

    const size_t imagesize = params.width * params.height;


    if( 0 == hdr_compression_flag ) {

        /* NO COMPRESSION: of the original image.
         */
        if( imagesize * sizeof(uint16_t) != hdr_compressed_size_bytes )

            return ErrBadImageData;  // inconsistent size of the uncompressed
                                     // data block.

        size_t addr = 0;
        str << "------"              << "----------------------" << "-"                  << "-------------------\n";
        str << ::format_addr( addr ) << "  [ Compression Flag: " << hdr_compression_flag << " (NO COMPRESSION) ]\n";
        addr += sizeof(uint32_t);
        str << ::format_addr( addr ) << "  [ Original Checksum: " << hdr_original_checksum << " ]\n";
        addr += sizeof(uint32_t);
        str << ::format_addr( addr ) << "  [ ImageParams: " << params.width << "," << params.height << "," << params.depth << " (width,height,depth) ]\n";
        addr += sizeof(uint32_t) * 3;
        str << ::format_addr( addr ) << "  [ Image Size: " << hdr_compressed_size_bytes << " (bytes) ]\n";
        addr += sizeof(uint32_t);
        str << ::format_addr( addr ) << "  " << ::format_addr( addr - hdr_size_bytes ) << "  [ <original image> ]\n";
        addr += hdr_compressed_size_bytes - 1;
        str << "......"              << "  " << "......"                               << "  [ <original image> ]\n";
        str << ::format_addr( addr ) << "  " << ::format_addr( addr - hdr_size_bytes ) << "  [ <original image> ]\n";

        return Success;
    }

    /* COMPRESSED IMAGE: expect a bitmap (compressed or not) attached
     * after a compressed image.
     */
    const uint16_t base            = *(uint16_t*)hdr_ptr; hdr_ptr += sizeof(uint16_t);
    const size_t   data_size_bytes = *(uint32_t*)hdr_ptr; hdr_ptr += sizeof(uint32_t);

    if( hdr_compressed_size_bytes <
        sizeof(uint16_t) +
        sizeof(uint32_t) + data_size_bytes )

        return ErrBadImageData;  // inconsistent size of the compressed
                                 // data block.

    uint8_t*     data_ptr           = hdr_ptr;
    uint8_t*     bitmap_size_ptr    = data_ptr + data_size_bytes;
    const size_t bitmap_size_shorts = *(uint32_t*)bitmap_size_ptr; bitmap_size_ptr += sizeof(uint32_t);
    //uint16_t*    bitmap_ptr         = (uint16_t*)bitmap_size_ptr;

    if( hdr_compressed_size_bytes !=
        sizeof(uint16_t) +                      // base
        sizeof(uint32_t) +                      // compressed image itself
        data_size_bytes  +
        sizeof(uint32_t) +
        sizeof(uint16_t) * bitmap_size_shorts )

            return ErrBadImageBitMap;  // inconsistent size of the bitmap
                                       // data block.

    size_t addr = 0;

    if( 3 == hdr_compression_flag ) {
        str << "------"              << "----------------------" << "-"                  << "-----------------------------------------\n";
        str << ::format_addr( addr ) << "  [ Compression Flag: " << hdr_compression_flag << " (COMPRESSED IMAGE + COMPRESSED BITMAP) ]\n";
    } else if( 1 == hdr_compression_flag ) {
        str << "------"              << "----------------------" << "-"                  << "---------------------------------------------\n";
        str << ::format_addr( addr ) << "  [ Compression Flag: " << hdr_compression_flag << " (COMPRESSED IMAGE + NON-COMPRESSED BITMAP) ]\n";
    } else {
        return ErrUnknownComprMethod;  // unsupported compresion algorithm
    }

    addr += sizeof(uint32_t);
    str << ::format_addr( addr ) << "  [ Original Image Checksum: " << hdr_original_checksum << " ]\n";
    addr += sizeof(uint32_t);
    str << ::format_addr( addr ) << "  [ ImageParams: " << params.width << "," << params.height << "," << params.depth << " (width,height,depth) ]\n";
    addr += sizeof(uint32_t) * 3;
    str << ::format_addr( addr ) << "  [ Image + Bitmap Size: " << hdr_compressed_size_bytes << " (bytes) ]\n";
    addr += sizeof(uint32_t);
    size_t addr_image_bitmap = addr;
    str << ::format_addr( addr ) << "  " << ::format_addr( addr - addr_image_bitmap ) << "  [ Base: " << base << " ]\n";
    addr += sizeof(uint16_t);
    str << ::format_addr( addr ) << "  " << ::format_addr( addr - addr_image_bitmap ) << "  [ Compressed Image Size: " << data_size_bytes << " (bytes) ]\n";
    addr += sizeof(uint32_t);
    size_t addr_image = addr;
    str << ::format_addr( addr ) << "  " << ::format_addr( addr - addr_image_bitmap ) << "  " << ::format_addr( addr - addr_image ) << "  [ <compressed image> ]\n";
    addr += data_size_bytes - 1;
    str << "......"              << "  " << "......"                                  << "  " << "......"  <<                          "  [ <compressed image> ]\n";
    str << ::format_addr( addr ) << "  " << ::format_addr( addr - addr_image_bitmap ) << "  " << ::format_addr( addr - addr_image ) << "  [ <compressed image> ]\n";
    addr += 1;
    str << ::format_addr( addr ) << "  " << ::format_addr( addr - addr_image_bitmap ) << "  [ Bitmap Size: " << bitmap_size_shorts << " (16-bit words) or " << sizeof(uint16_t) * bitmap_size_shorts << " (bytes) ]\n";
    addr += sizeof(uint32_t);
    str << ::format_addr( addr ) << "  " << ::format_addr( addr - addr_image_bitmap ) << "  " << ::format_addr( 0 ) << "  [ <bitmap> ]\n";
    addr += sizeof(uint16_t) * bitmap_size_shorts - 1;
    str << "......"              << "  " << "......" <<                                  "  " << "......" <<           "  [ <bitmap> ]\n";
    str << ::format_addr( addr ) << "  " << ::format_addr( addr - addr_image_bitmap ) << "  " << ::format_addr( sizeof(uint16_t) * bitmap_size_shorts - 1 ) << "  [ <bitmap> ]\n";

    return Success;
}

const char*
CspadCompressor::err2str(int code)
{
    switch(code) {
    case Success:               return "Success";
    case ErrZeroImage:          return "Zero pointer instead of a compressed or uncompressed image";
    case ErrIllegalDepth:       return "Illegal/unsupported image depth";
    case ErrIllegalWidth:       return "Illegal/unsupported image width";
    case ErrIllegalHeight:      return "Illegal/unsupported image height";
    case ErrLargeImage:         return "Image is is too large";
    case ErrSmallImage:         return "Image is is too small";
    case ErrBadHdr:             return "Bad image header";
    case ErrBadImageData:       return "Inconsistent size of the image data block";
    case ErrBadImageBitMap:     return "Inconsistent size of the bitmap data block";
    case ErrCSMissmatch:        return "Checksum doesn't match the original one";
    case ErrBitMapMissmatch:    return "Bit-map isn't consistent with the compressed image size";
    case ErrUnknownComprMethod: return "Unknown compresion method";
    }
    return "Unknown status code";
}


}  // namespace Codec
}  // namespace Pds
