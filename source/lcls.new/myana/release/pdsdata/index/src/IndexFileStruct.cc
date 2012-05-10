#include "pdsdata/index/IndexList.hh"

namespace Pds
{  
namespace Index
{

/*
 * class IndexFileHeaderV1
 */ 
IndexFileHeaderV1::IndexFileHeaderV1(const IndexList& list) :
  typeId(TypeId::Id_Index, iXtcIndexVersion)
{
  strncpy(sXtcFilename, list._sXtcFilename, iMaxFilenameLen-1);    
  sXtcFilename[iMaxFilenameLen-1] = 0;  
  iNumCalib       = list._lCalib.size();
  iNumEvrEvents   = list._mapEvrToId.size();
  iNumDetector    = list._iNumSegments;  
  iNumIndex       = list._lNode.size();
}

/*
 * class IndexFileHeaderV2
 */ 
IndexFileHeaderV2::IndexFileHeaderV2(const IndexList& list) :
  typeId(TypeId::Id_Index, iXtcIndexVersion)
{
  strncpy(sXtcFilename, list._sXtcFilename, iMaxFilenameLen-1);    
  sXtcFilename[iMaxFilenameLen-1] = 0;  
  iNumCalib       = list._lCalib.size();
  iNumEvrEvents   = list._mapEvrToId.size();
  iNumDetector    = list._iNumSegments;  
  iNumIndex       = list._lNode.size();
  iNumOutOrder    = list._iNumOutOrder;
  iNumOverlapPrev = list._iNumOverlapPrev;
  iNumOverlapNext = list._iNumOverlapNext;  
}

IndexFileHeaderV2::IndexFileHeaderV2(const IndexFileHeaderV1& headerV1) :
  typeId(TypeId::Id_Index, iXtcIndexVersion),
  iNumCalib       (headerV1.iNumCalib),
  iNumEvrEvents   (headerV1.iNumEvrEvents),
  iNumDetector    (headerV1.iNumDetector),
  iNumIndex       (headerV1.iNumIndex),
  iNumOutOrder    (0),
  iNumOverlapPrev (0),
  iNumOverlapNext (0) 
{  
  strncpy( sXtcFilename, headerV1.sXtcFilename, iMaxFilenameLen-1);    
  sXtcFilename[iMaxFilenameLen-1] = 0;  
}

/*
 * class IndexFileL1NodeV1
 */ 
IndexFileL1NodeV1::IndexFileL1NodeV1(const L1AcceptNode& node) :
  uSeconds        (node.uSeconds),
  uNanoseconds    (node.uNanoseconds),
  uFiducial       (node.uFiducial), 
  i64OffsetXtc    (node.i64OffsetXtc), 
  damage          (node.damage),
  uMaskDetDmgs    (node.uMaskDetDmgs),
  uMaskDetData    (node.uMaskDetData),
  uMaskEvrEvents  (node.uMaskEvrEvents)
{  
}

int convertTimeStringToSeconds(const char* sTime, uint32_t& uSeconds, uint32_t& uNanoseconds)
{  
  uSeconds      = 0;
  uNanoseconds  = 0;
  if (sTime == NULL)
    return 1;
    
  struct tm tm;
    
  strptime(sTime, "%Y-%m-%d %H:%M:%S", &tm);
  if (tm.tm_year < 0) tm.tm_year += 2000;
  
  char* pDST = strstr(sTime, "PDT");
  if (pDST == NULL)
    pDST = strstr(sTime, "DST");
  if (pDST != NULL)
      tm.tm_isdst = 1;
  else
      tm.tm_isdst = 0;    
  
  char* pUTC = strstr(sTime, "UTC");
  if (pUTC == NULL)
    pUTC = strstr(sTime, "GMT");
  if (pUTC == NULL)
    pUTC = strstr(sTime, "Z");
  
  if (pUTC != NULL) 
  {
    tm.tm_isdst = 0;
    tm.tm_hour += 8;
  }
  
  uSeconds = mktime(&tm);
  
  char* pDot = strchr(sTime, '.');
  if ( pDot != NULL)
  {
    double fNanoseconds = strtod(pDot, NULL);
    uNanoseconds = (uint32_t)(fNanoseconds * 1e9 + 0.5);
  }
  
  char sTimeBuff[128];
  strftime(sTimeBuff, 128, "%Z %a %F %T", &tm);
  printf("Time %s.%03us%s (seconds 0x%x nanosecs 0x%x)\n",
    sTimeBuff, (int)(uNanoseconds/1e6), 
    ( ( pDST != NULL ) ? " (adjusted by DST)" : "" ),
    uSeconds, uNanoseconds);    
    
  return 0;
}

} // namespace Index
} // namespace Pds
