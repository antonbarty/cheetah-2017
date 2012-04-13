#ifndef IpmFexConfigV2_hh
#define IpmFexConfigV2_hh

#include "pdsdata/lusi/DiodeFexConfigV2.hh"

namespace Pds {
  namespace Lusi {
#pragma pack(4)
    class IpmFexConfigV2 {
    public:
      enum {Version=2};
      IpmFexConfigV2 () {}
      ~IpmFexConfigV2 () {}
      IpmFexConfigV2 (const DiodeFexConfigV2 _diode[],
		      float _xscale,
		      float _yscale);
    public:
      enum { NCHANNELS=4 };
      DiodeFexConfigV2 diode[NCHANNELS];
      float xscale;
      float yscale;
    };
#pragma pack()
  }
}

#endif
