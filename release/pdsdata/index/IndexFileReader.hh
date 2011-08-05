#ifndef Pds_Index_IndexFileReader_hh
#define Pds_Index_IndexFileReader_hh

#include <stdio.h>
#include <vector>

#include "pdsdata/xtc/ProcInfo.hh"

#include "IndexFileStruct.hh"

namespace Pds
{  
namespace Index
{
  
#pragma pack(1)
class IndexFileReader
{
public:
      IndexFileReader ();
      ~IndexFileReader();  
  
  int   open          (const char* sXtcIndex);
  int   close         ();
  bool  isValid       () const; 

  int numL1Event        (int& iNumL1Event) const;
  int detectorList      (int& iNumDetector, const ProcInfo*& lDetector) const;
  int srcList           (int iDetector, int& iNumSrc, const Src*& lSrc) const;
  int typeList          (int iDetector, int& iNumType, const TypeId*& lType) const;
  int numCalibCycle     (int& iNumCalib) const;
  int calibCycleList    (const CalibNode*& lCalib) const;
  int numL1EventInCalib (int iCalib       , int& iNumL1Event) const;
  int eventCalibToGlobal(int iCalib, int iEvent, int& iGlobalEvent) const;
  int eventGlobalToCalib(int iGlobalEvent, int& iCalib, int& iEvent) const;
  int eventTimeToGlobal (uint32_t uSeconds, uint32_t uNanoseconds, int& iGlobalEvent, bool& bExactMatch, bool& bOvertime);
  int eventTimeToCalib  (uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime);
  
  int gotoEvent         (int iCalib, int iEvent, int64_t& i64Offset, int& iGlobalEvent);
  int gotoEventInXtc    (int iCalib, int iEvent, int fdXtc, int& iGlobalEvent);
  int gotoTimeInXtc     (uint32_t uSeconds, uint32_t uNanoseconds, int fdXtc, int& iGlobalEvent, bool& bExactMatch, bool& bOvertime);
    
  int time            (int iGlobalEvent, uint32_t& uSeconds, uint32_t& uNanoseconds);
  int fiducial        (int iGlobalEvent, uint32_t& uFiducial);
  int offset          (int iGlobalEvent, int64_t&  i64Offset);
  int damage          (int iGlobalEvent, Damage& damage);
  int detDmgMask      (int iGlobalEvent, uint32_t& uMaskDetDmgs);
  int detDataMask     (int iGlobalEvent, uint32_t& uMaskDetData);
  int evrEventList    (int iGlobalEvent, unsigned int& uNumEvent, const uint8_t*& lEvrEvent);

private:
  int gotoL1Node      (int iL1Node);
  
  typedef std::vector<TypeId>     TTypeList;
  typedef std::vector<Src>        TSrcList;
  typedef std::vector<TTypeList>  TListOfTypeList;
  typedef std::vector<TSrcList>   TListOfSrcList;
  
  int                 _fdXtcIndex;
  IndexFileHeaderType _fileHeader;
  std::vector<CalibNode>
                      _lCalib;
  std::vector<uint8_t>
                      _lEvrEvent;
  std::vector<ProcInfo>
                      _lDetector;
  TListOfTypeList     _lTypeList;
  TListOfSrcList      _lSrcList;
  
  int                 _iCurL1Node;
  IndexFileL1NodeType _curL1Node;  
    
  std::vector<uint8_t>
                      _lCurEvrEvent;

public:

  static inline int compareTime(uint32_t& uSeconds1, uint32_t& uNanoseconds1, uint32_t& uSeconds2, uint32_t& uNanoseconds2)
  {
    if (uSeconds1 > uSeconds2) return 1;
    if (uSeconds1 < uSeconds2) return -1;  
    
    // remaining case: (uSeconds1 == uSeconds2)
    if (uNanoseconds1 > uNanoseconds2) return 1;
    if (uNanoseconds1 < uNanoseconds2) return -1;  
    return 0;  
  }
};

#pragma pack()

} // namespace Index
} // namespace Pds

#endif // #ifndef Pds_Index_IndexFileReader_hh
