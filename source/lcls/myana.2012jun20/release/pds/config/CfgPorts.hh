#ifndef Pds_CfgPorts_hh
#define Pds_CfgPorts_hh

#include "pds/service/Ins.hh"

namespace Pds {
  class CfgPorts {
  public:
    static Ins ins(unsigned platform);
  };
}

#endif
