#include "pdsdata/index/IndexSliceReader.hh"

namespace Pds
{  
namespace Index
{

int IndexSliceReader::open(const char* sXtcIndex)
{
  return 0;
}

int IndexSliceReader::close()
{
  return 0;
}

bool IndexSliceReader::isValid() const 
{
  return 0;
}

int IndexSliceReader::numL1Event(int& iNumL1Event) const
{
  return 0;
}

int IndexSliceReader::detectorList(int& iNumDetector, const ProcInfo*& lDetector) const
{
  return 0;
}

int IndexSliceReader::srcList(int iDetector, int& iNumSrc, const Src*& lSrc) const
{
  return 0;
}

int IndexSliceReader::typeList(int iDetector, int& iNumType, const TypeId*& lType) const
{
  return 0;
}

int IndexSliceReader::calibCycleList(int& iNumCalib, const CalibNode*& lCalib) const
{
  return 0;
}

int IndexSliceReader::numL1EventInCalib(int iCalib, int& iNumL1Event) const
{
  return 0;
}

int IndexSliceReader::eventLocalToGlobal(int iCalib, int iEvent, int& iGlobalEvent) const
{
  return 0;
}

int IndexSliceReader::eventGlobalToLocal(int iGlobalEvent, int& iCalib, int& iEvent) const
{
  return 0;
}

int IndexSliceReader::eventTimeToGlobal(uint32_t uSeconds, uint32_t uNanoseconds, int& iGlobalEvent, bool& bExactMatch, bool& bOvertime)
{
  return 0;
}

int IndexSliceReader::eventTimeToLocal(uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime)
{
  return 0;
}

int IndexSliceReader::gotoEvent(int iCalib, int iEvent, int64_t& i64Offset, int& iGlobalEvent)
{
  return 0;
}

int IndexSliceReader::gotoEventAndSeek(int iCalib, int iEvent, int fdXtc, int& iGlobalEvent)
{
  return 0;
}

int IndexSliceReader::gotoTime(uint32_t uSeconds, uint32_t uNanoseconds, int fdXtc, int& iGlobalEvent, bool& bExactMatch, bool& bOvertime)
{
  return 0;
}

int IndexSliceReader::time(int iEvent, uint32_t& uSeconds, uint32_t& uNanoseconds)
{
  return 0;
}

int IndexSliceReader::fiducial(int iEvent, uint32_t& uFiducial)
{
  return 0;
}

int IndexSliceReader::damage(int iEvent, Damage& damage)
{
  return 0;
}

int IndexSliceReader::detDmgMask(int iEvent, uint32_t& uMaskDetDmgs)
{
  return 0;
}

int IndexSliceReader::detDataMask(int iEvent, uint32_t& uMaskDetData)
{
  return 0;
}

int IndexSliceReader::evrEventList(int iEvent, unsigned int& uNumEvent, const uint8_t*& lEvrEvent)
{
  return 0;
}
  
}//namespace Index
}//namespace Pds
