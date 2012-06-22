#ifndef Pds_BldInfo_hh
#define Pds_BldInfo_hh

#include <stdint.h>
#include "pdsdata/xtc/Src.hh"

namespace Pds {

  class Node;

  class BldInfo : public Src {
  public:

    enum Type { EBeam, PhaseCavity, FEEGasDetEnergy, Nh2Sb1Ipm01,  
                HxxUm6Imb01, HxxUm6Imb02,
                HfxDg2Imb01, HfxDg2Imb02,
                XcsDg3Imb03, XcsDg3Imb04,
                HfxDg3Imb01, HfxDg3Imb02,
                HxxDg1Cam,   HfxDg2Cam,
                HfxDg3Cam,   XcsDg3Cam,
                HfxMonCam,
                HfxMonImb01, HfxMonImb02,
                HfxMonImb03,
                MecLasEm01, MecTctrPip01,
                MecTcTrDio01,
                MecXt2Ipm02, MecXt2Ipm03, 
                MecHxmIpm01,
                NumberOf };

    BldInfo() {}
    BldInfo(uint32_t processId,
            Type     type);

    uint32_t processId() const;
    Type     type()  const;

    static const char* name(const BldInfo&);
  };

}
#endif
