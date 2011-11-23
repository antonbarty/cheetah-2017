#ifndef DiodeFexV1_hh
#define DiodeFexV1_hh

namespace Pds {
  namespace Lusi {
#pragma pack(4)
    class DiodeFexV1 {
    public:
      enum {Version=1};
      DiodeFexV1() {}
      DiodeFexV1( float ch0 );
    public:
      float value;
    };
#pragma pack()
  }
}

#endif
