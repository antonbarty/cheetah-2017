#ifndef Pds_CompressedPayload_hh
#define Pds_CompressedPayload_hh

#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace Pds {

  class CompressedPayload {
  public:
    enum Engine {
      None   = 0,
      Hist16 = 1,
      HistN  = 2,
    };

    CompressedPayload();
    CompressedPayload(Engine   engine,
                      unsigned decomp_size,
                      unsigned comp_size);
  public:
    Engine compressor() const;
    unsigned dsize() const; // decompressed size
    unsigned csize() const; // compressed size
    const void* cdata() const; // pointer to compressed data
  public:
    bool uncompress(void* outbuf) const;
  private:
    uint32_t _engine;
    uint32_t _reserved;
    uint32_t _dsize;
    uint32_t _csize;
  };

};

#endif
