#ifndef DiodeFexConfigV2_hh
#define DiodeFexConfigV2_hh

namespace Pds {
  namespace Lusi {
#pragma pack(4)
    class DiodeFexConfigV2 {
    public:
      enum {Version=2};
      DiodeFexConfigV2 () {}
      ~DiodeFexConfigV2 () {}
      DiodeFexConfigV2 (float _base [],
			float _scale[]);
    public:
      enum { NRANGES=16 };
      float base [NRANGES];
      float scale[NRANGES];
    };
#pragma pack()
  }
}

#endif
