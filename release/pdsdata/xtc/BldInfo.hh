#ifndef Pds_BldInfo_hh
#define Pds_BldInfo_hh

#include <stdint.h>
#include "pdsdata/xtc/Src.hh"

namespace Pds {

  class Node;

  class BldInfo : public Src {
  public:

    enum Type { EBeam, PhaseCavity, FEEGasDetEnergy, Nh2Sb1Ipm01,  NumberOf };

    BldInfo() {}
    BldInfo(uint32_t processId,
        Type     type);

    uint32_t processId() const;
    Type     type()  const;

    static const char* name(const BldInfo&);
  };

}
#endif
