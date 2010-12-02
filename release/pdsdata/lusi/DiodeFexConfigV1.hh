#ifndef DiodeFexConfigV1_hh
#define DiodeFexConfigV1_hh

namespace Pds {
  namespace Lusi {
#pragma pack(4)
    class DiodeFexConfigV1 {
    public:
      enum {Version=1};
      DiodeFexConfigV1 () {}
      ~DiodeFexConfigV1 () {}
      DiodeFexConfigV1 (float _base [],
			float _scale[]);
    public:
      enum { NRANGES=3 };
      float base [NRANGES];
      float scale[NRANGES];
    };
#pragma pack()
  }
}

#endif
