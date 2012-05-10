#ifndef IpmFexV1_hh
#define IpmFexV1_hh

namespace Pds {
  namespace Lusi {
#pragma pack(4)
    class IpmFexV1 {
    public:
      enum {Version=1};
      IpmFexV1() {}
      IpmFexV1( float ch[],
		float x, float y, float chsum );
    public:
      float channel[4];
      float sum;
      float xpos;
      float ypos;
    };
#pragma pack()
  }
}

#endif
