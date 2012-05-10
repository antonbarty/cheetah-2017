#ifndef IpmFexConfigV1_hh
#define IpmFexConfigV1_hh

#include "pdsdata/lusi/DiodeFexConfigV1.hh"

namespace Pds {
  namespace Lusi {
#pragma pack(4)
    class IpmFexConfigV1 {
    public:
      enum {Version=1};
      IpmFexConfigV1 () {}
      ~IpmFexConfigV1 () {}
      IpmFexConfigV1 (const DiodeFexConfigV1 _diode[],
		      float _xscale,
		      float _yscale);
    public:
      enum { NCHANNELS=4 };
      DiodeFexConfigV1 diode[NCHANNELS];
      float xscale;
      float yscale;
    };
#pragma pack()
  }
}

#endif
