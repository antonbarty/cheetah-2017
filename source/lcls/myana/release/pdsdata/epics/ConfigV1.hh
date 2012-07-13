#ifndef EPICS_CONFIG_V1_HH
#define EPICS_CONFIG_V1_HH

#include <pdsdata/xtc/Xtc.hh>
#include <stdint.h>

#pragma pack(4)

namespace Pds 
{

namespace Epics
{

struct PvConfigV1
{    
  static const int iMaxPvDescLength = 64;
  
  int16_t iPvId;
  char    sPvDesc[iMaxPvDescLength];
  float   fInterval;
  
  PvConfigV1() {}
  PvConfigV1(int iPvId1, const char* sPvDesc1, float fInterval1);
};
  
/*
 * This class is an expanding class
 *
 * An PvConfig array is attached to the end of this class. 
 * The length of array is defined by _iNumPv
 */  
class ConfigV1 
{
public:
  enum { Version = 1 };

  ConfigV1()  {}  
  explicit ConfigV1(int iNumPv); // explicit to disable auto conversion from ints
  
  typedef PvConfigV1 PvConfig;

  int         getNumPv()            const { return _iNumPv; }
  PvConfig*   getPvConfig(int iPv)  const { return (PvConfig*)(this+1) + iPv; }
  
  int size()      const { return sizeof(*this) + _iNumPv * sizeof(PvConfig); } 
  
private:  
  int32_t _iNumPv;    
};

} // namespace Epics

} // namespace Pds 

#pragma pack()

#endif
