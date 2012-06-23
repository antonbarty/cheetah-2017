#include "pdsdata/xtc/BldInfo.hh"
#include "pdsdata/xtc/Level.hh"
#include <stdint.h>
#include <stdio.h>

using namespace Pds;

BldInfo::BldInfo(uint32_t processId, Type type) : Src(Level::Reporter) {
  _log |= processId&0x00ffffff;
  _phy = type;
}

uint32_t BldInfo::processId() const { return _log&0xffffff; }

BldInfo::Type BldInfo::type() const {return (BldInfo::Type)(_phy); }

const char* BldInfo::name(const BldInfo& src){
  static const char* _typeNames[] = {
    "EBeam",
    "PhaseCavity",
    "FEEGasDetEnergy",
    "NH2-SB1-IPM-01",
    "XCS-IPM-01",
    "XCS-DIO-01",
    "XCS-IPM-02",
    "XCS-DIO-02",
    "XCS-IPM-03",
    "XCS-DIO-03",
    "XCS-IPM-03m",
    "XCS-DIO-03m",
    "XCS-YAG-1",
    "XCS-YAG-2",
    "XCS-YAG-3m",
    "XCS-YAG-3",
    "XCS-YAG-mono", 
    "XCS-IPM-mono",
    "XCS-DIO-mono",
    "XCS-DEC-mono",
    "MEC-LAS-EM-01",
    "MEC-TCTR-PIP-01",
    "MEC-TCTR-DI-01",
    "MEC-XT2-IPM-02",
    "MEC-XT2-IPM-03",
    "MEC-HXM-IPM-01",
  };
  return (src.type() < NumberOf ? _typeNames[src.type()] : "-Invalid-");
}
