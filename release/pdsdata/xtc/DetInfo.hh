#ifndef Pds_DetInfo_hh
#define Pds_DetInfo_hh

#include <stdint.h>
#include "pdsdata/xtc/Src.hh"

namespace Pds {
  class Node;

  class DetInfo:public Src {
  public:
    /*
     * Notice: New enum values should be appended to the end of the enum list, since
     *   the old values have already been recorded in the existing xtc files. 
     */
    enum Detector {
      NoDetector    = 0,
      AmoIms        = 1,
      AmoGasdet     = 2,
      AmoETof       = 3,
      AmoITof       = 4,
      AmoMbes       = 5,
      AmoVmi        = 6,
      AmoBps        = 7,
      Camp          = 8,
      EpicsArch     = 9,
      BldEb         = 10,
      SxrBeamline   = 11,
      SxrEndstation = 12,
      XppSb1Ipm     = 13,
      XppSb1Pim     = 14,
      XppMonPim     = 15,
      XppSb2Ipm     = 16,
      XppSb3Ipm     = 17,
      XppSb3Pim     = 18,
      XppSb4Pim     = 19,
      XppGon        = 20,
      XppLas        = 21,
      XppEndstation = 22,
      AmoEndstation = 23,
      CxiEndstation = 24,
      XcsEndstation = 25,
      MecEndstation = 26,
      NumDetector   = 27
    };

    enum Device {
      NoDevice  = 0,
      Evr       = 1,
      Acqiris   = 2,
      Opal1000  = 3,
      TM6740    = 4,
      pnCCD     = 5,
      Princeton = 6,
      Fccd      = 7,
      Ipimb     = 8,
      Encoder   = 9,
      Cspad     = 10,
      NumDevice = 11
    };

    DetInfo() {}
    DetInfo(uint32_t processId, Detector det, uint32_t detId, Device dev, uint32_t devId);

    bool operator==(const DetInfo &) const;

    uint32_t processId() const;
    Detector detector() const;
    Device device() const;
    uint32_t detId() const;
    uint32_t devId() const;

    static const char *name(Detector);
    static const char *name(Device);
    static const char *name(const DetInfo &);
  };

}
#endif
