#ifndef PimImageConfigV1_hh
#define PimImageConfigV1_hh

namespace Pds {
  namespace Lusi {
#pragma pack(4)
    class PimImageConfigV1 {
    public:
      enum {Version=1};
      PimImageConfigV1 () {}
      ~PimImageConfigV1 () {}
      PimImageConfigV1 (float _xscale, float _yscale);
    public:
      float xscale;
      float yscale;
    };
#pragma pack()
  }
}

#endif
