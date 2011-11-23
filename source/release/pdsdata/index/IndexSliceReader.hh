#ifndef Pds_IndexSliceReader_hh
#define Pds_IndexSliceReader_hh

#include "pdsdata/index/IndexFileReader.hh"

namespace Pds
{  
namespace Index
{

class IndexSliceReader
{
      IndexSliceReader();
      ~IndexSliceReader();  
  
  int   open          (const char* sXtcIndex);
  int   close         ();
  bool  isValid       () const; 

  int numL1Event        (int& iNumL1Event) const;
  int detectorList      (int& iNumDetector, const ProcInfo*& lDetector) const;
  int srcList           (int iDetector, int& iNumSrc, const Src*& lSrc) const;
  int typeList          (int iDetector, int& iNumType, const TypeId*& lType) const;
  int calibCycleList    (int& iNumCalib   , const CalibNode*& lCalib) const;
  int numL1EventInCalib (int iCalib       , int& iNumL1Event) const;
  int eventLocalToGlobal(int iCalib, int iEvent, int& iGlobalEvent) const;
  int eventGlobalToLocal(int iGlobalEvent, int& iCalib, int& iEvent) const;
  int eventTimeToGlobal (uint32_t uSeconds, uint32_t uNanoseconds, int& iGlobalEvent, bool& bExactMatch, bool& bOvertime);
  int eventTimeToLocal  (uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime);
  
  int gotoEvent         (int iCalib, int iEvent, int64_t& i64Offset, int& iGlobalEvent);
  int gotoEventAndSeek  (int iCalib, int iEvent, int fdXtc, int& iGlobalEvent);
  int gotoTime          (uint32_t uSeconds, uint32_t uNanoseconds, int fdXtc, int& iGlobalEvent, bool& bExactMatch, bool& bOvertime);
  
  int time            (int iEvent, uint32_t& uSeconds, uint32_t& uNanoseconds);
  int fiducial        (int iEvent, uint32_t& uFiducial);
  int damage          (int iEvent, Damage& damage);
  int detDmgMask      (int iEvent, uint32_t& uMaskDetDmgs);
  int detDataMask     (int iEvent, uint32_t& uMaskDetData);
  int evrEventList    (int iEvent, unsigned int& uNumEvent, const uint8_t*& lEvrEvent);

private:
  int gotoL1Node      (int iL1Node);
  
};
  
}//namespace Index
}//namespace Pds
 

#endif //#ifndef Pds_IndexSliceReader_hh
