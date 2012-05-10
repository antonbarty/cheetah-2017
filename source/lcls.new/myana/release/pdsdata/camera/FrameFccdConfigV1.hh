//
//  class for configuring FCCD frame
//
#ifndef Camera_FrameFccdConfigV1_hh
#define Camera_FrameFccdConfigV1_hh

#include "pdsdata/camera/FrameCoord.hh"

namespace Pds {
  namespace Camera {

    class FrameFccdConfigV1 {
    public:
      enum { Version=1 };
      FrameFccdConfigV1();
      FrameFccdConfigV1(const FrameFccdConfigV1&);
      //  size of this structure
      unsigned  size()        const         { return sizeof(*this); }
    };

  };
};
#endif
