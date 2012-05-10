#include "pdsdata/epics/ConfigV1.hh"

#include <string.h>

namespace Pds 
{

namespace Epics
{ 
  
ConfigV1::ConfigV1(int iNumPv) : _iNumPv(iNumPv)  
{
}  

PvConfigV1::PvConfigV1(int iPvId1, const char* sPvDesc1, float fInterval1) :
  iPvId(iPvId1), fInterval(fInterval1)
{
  strncpy(sPvDesc, sPvDesc1, iMaxPvDescLength-1);
  sPvDesc[iMaxPvDescLength - 1] = 0;
}

} // namespace Epics

} // namespace Pds 
