#ifndef Ami_ChannelID_hh
#define Ami_ChannelID_hh

#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {
  class ChannelID {
  public:
    static const char* name(const Pds::DetInfo& info, unsigned channel=0);
  };
};

#endif
