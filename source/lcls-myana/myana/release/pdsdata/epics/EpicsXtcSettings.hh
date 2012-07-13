#ifndef EPICS_XTC_SETTINGS_H
#define EPICS_XTC_SETTINGS_H
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace Pds
{
    
namespace EpicsXtcSettings
{
  const int            iXtcVersion = 1;    
  const int            iMaxNumPv   = 10000; 
  
  /*
   * 200 Bytes: For storing a DBR_CTRL_DOUBLE PV
   */
  const int            iMaxXtcSize = iMaxNumPv * 200;
  const TypeId::Type   typeXtc     = TypeId::Id_Epics;
  extern const DetInfo detInfo;
}

}

#endif
