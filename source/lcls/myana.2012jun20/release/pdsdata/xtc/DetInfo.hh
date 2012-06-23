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
      CxiDg1        = 27,
      CxiDg2        = 28,
      CxiDg3        = 29,
      CxiDg4        = 30,
      CxiKb1        = 31,
      CxiDs1        = 32,
      CxiDs2        = 33,
      CxiDsu        = 34,
      CxiSc1        = 35,
      CxiDsd        = 36,
      XcsBeamline   = 37,
      CxiSc2        = 38,
      MecXuvSpectrometer = 39,
      MecXrtsForw   = 40,
      MecXrtsBack   = 41,
      MecFdi        = 42,
      MecTimeTool   = 43,
      MecTargetChamber = 44,
      NumDetector   = 45
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
      AcqTDC    = 11,
      Xamps     = 12,
      Cspad2x2  = 13,
      Fexamp    = 14,
      Gsc16ai   = 15,
      Phasics   = 16,
      Timepix   = 17,
      Opal2000  = 18,
      Opal4000  = 19,
      OceanOptics = 20,
      Opal1600  = 21,
      Opal8000  = 22,
      Fli       = 23,
      NumDevice = 24
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
