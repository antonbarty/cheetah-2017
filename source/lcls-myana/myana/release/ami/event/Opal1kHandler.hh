#ifndef Ami_Opal1kHandler_hh
#define Ami_Opal1kHandler_hh

#include "ami/event/FrameHandler.hh"

namespace Ami {
  class Opal1kHandler : public FrameHandler {
  public:
    Opal1kHandler(const Pds::DetInfo& info);
  };
};

#endif
