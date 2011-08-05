#include <errno.h>
#include <fcntl.h>

#include "pdsdata/index/IndexFileReader.hh"

namespace Pds
{  
namespace Index
{
  
IndexFileReader::IndexFileReader() : _fdXtcIndex(-1), _iCurL1Node(-1)
{
}

IndexFileReader::~IndexFileReader()
{
  close();
}

int IndexFileReader::open(const char* sXtcIndex)
{
  close();
    
  _fdXtcIndex = ::open(sXtcIndex, O_RDONLY | O_LARGEFILE);  
  if ( _fdXtcIndex == -1 )
  {
    printf( "IndexFileReader::open(): Open index file %s failed, error = %s\n",
      sXtcIndex, strerror(errno) );
    return 1;
  }
    
  int iRead = ::read(_fdXtcIndex, &_fileHeader, sizeof(_fileHeader));
  if ( iRead != sizeof(_fileHeader) )
  {
    printf( "IndexFileReader::open(): Read header failed. Read %d bytes (expected %d), error = %s\n",
      iRead, sizeof(_fileHeader), strerror(errno) );
    return 2;
  }
  
  /*
   * Calculate the file offset to CalibCycle
   */
  int64_t i64OffsetCalib = sizeof(_fileHeader) + sizeof(_curL1Node)* (int64_t) _fileHeader.iNumIndex;
  
  int64_t i64OffsetSeek = ::lseek64(_fdXtcIndex, i64OffsetCalib, SEEK_SET);
  if ( i64OffsetSeek != i64OffsetCalib )
  {
    printf("IndexFileReader::open(): Seek to CalibCycle offset 0x%Lx failed, result = 0x%Lx\n", 
      i64OffsetCalib, i64OffsetSeek );
  }

  if (_fileHeader.iNumCalib > 0)
  {
    _lCalib.resize(_fileHeader.iNumCalib);
    const int iReadSize = _lCalib.size() * sizeof(_lCalib[0]);
    iRead = ::read(_fdXtcIndex, (void*) &_lCalib[0], iReadSize);
    if (iRead != iReadSize)
    {
      printf( "IndexFileReader::open(): Read calib cycle (%d) failed. Read %d bytes (expected %d), error = %s\n",
        _fileHeader.iNumCalib, iRead, iReadSize, strerror(errno) );
      return 3;
    }  
  }

  if (_fileHeader.iNumEvrEvents > 0)
  {
    _lEvrEvent.resize(_fileHeader.iNumEvrEvents);
    const int iReadSize = _lEvrEvent.size() * sizeof(_lEvrEvent[0]);
    iRead = ::read(_fdXtcIndex, (void*) &_lEvrEvent[0], iReadSize);
    if (iRead != iReadSize)
    {
      printf( "IndexFileReader::open(): Read evr events (%d) failed. Read %d bytes (expected %d), error = %s\n",
        _fileHeader.iNumEvrEvents, iRead, iReadSize, strerror(errno) );
      return 4;
    }  
  }  
  
  if (_fileHeader.iNumDetector > 0)
  {
    _lDetector.resize(_fileHeader.iNumDetector, ProcInfo(Level::Segment,0,0));
    _lTypeList.resize(_fileHeader.iNumDetector);
    _lSrcList .resize(_fileHeader.iNumDetector);
    
    for ( int iDetector = 0; iDetector < _fileHeader.iNumDetector; ++iDetector )
    {
      int iReadSize = sizeof(_lDetector[iDetector]);
      iRead = ::read(_fdXtcIndex, (void*) &_lDetector[iDetector], iReadSize);
      if (iRead != iReadSize)
      {
        printf( "IndexFileReader::open(): Read detector %d procInfo failed. Read %d bytes (expected %d), error = %s\n",
          iDetector, iRead, iReadSize, strerror(errno) );
        return 5;
      }
      
      uint8_t uNumSrc;
      iReadSize = sizeof(uNumSrc);
      iRead = ::read(_fdXtcIndex, (void*)&uNumSrc, iReadSize);
      if (iRead != iReadSize)
      {
        printf( "IndexFileReader::open(): Read detector src# for detector %d failed. Read %d bytes (expected %d), error = %s\n",
          iDetector, iRead, iReadSize, strerror(errno) );
        return 6;
      }  

      TSrcList&  lSrc  = _lSrcList [iDetector];
      lSrc.resize(uNumSrc);
      iReadSize = lSrc.size() * sizeof(lSrc[0]);
      iRead = ::read(_fdXtcIndex, (void*) &lSrc[0], iReadSize);
      if ( iRead != iReadSize)
      {
        printf( "IndexFileReader::open(): Read src list for detector %d failed. Read %d bytes (expected %d), error = %s\n",
          iDetector, iRead, iReadSize, strerror(errno) );
        return 7;
      }              
      
      TTypeList& lType = _lTypeList[iDetector];
      lType.resize(uNumSrc);
      iReadSize = lType.size() * sizeof(lType[0]);
      iRead = ::read(_fdXtcIndex, (void*) &lType[0], iReadSize);
      if ( iRead != iReadSize)
      {
        printf( "IndexFileReader::open(): Read type list for detector %d failed. Read %d bytes (expected %d), error = %s\n",
          iDetector, iRead, iReadSize, strerror(errno) );
        return 8;
      }              
    } // for ( int iDetector = 0; iDetector < _fileHeader.iNumDetector; ++iDetector )
  } // if (_fileHeader.iNumDetector > 0)     
  
  return 0;
}

int IndexFileReader::close()
{
  if ( _fdXtcIndex == -1 )
    return 0;
    
  int iError = ::close(_fdXtcIndex);  
  if ( iError != 0 )
  {
    printf( "IndexFileReader::close(): Close index file failed, error = %s\n",
      strerror(errno) );
    return 1;
  }      
  
  _fdXtcIndex = -1;
  _iCurL1Node = -1;
  _lCalib   .clear();
  _lEvrEvent.clear();
  _lDetector.clear();
  _lSrcList .clear();
  _lTypeList.clear();
  
  return 0;
}

bool IndexFileReader::isValid() const
{
  return (_fdXtcIndex != -1);
}

int IndexFileReader::numL1Event(int& iNumL1Event) const
{
  iNumL1Event = -1;
  if (_fdXtcIndex == -1)
    return 1;       

  iNumL1Event = _fileHeader.iNumIndex;
  return 0;
}

int IndexFileReader::numL1EventInCalib(int iCalib, int& iNumL1Event) const
{
  iNumL1Event = -1;
  if (_fdXtcIndex == -1)
    return 1;
  
  if ( iCalib < -1 || iCalib >= (int) _lCalib.size() )
  {
    printf( "IndexFileReader::numL1EventInCalib(): Invalid Calib Cyle #%d (Max # = %d)\n",
      iCalib, _lCalib.size()-1 );
    return 2;
  }    

  /*
   * special case: iCalib == -1:
   *   1) _lCalib.size() == 0 
   *      iNumL1Event = _fileHeader.iNumIndex
   *   2) _lCalib.size() != 0 
   *      iNumL1Event = _lCalib[0].iL1Index
   */
  
  int iStartIndex = (iCalib >= 0 ? _lCalib[iCalib].iL1Index : 0);
  
  if ( iCalib == (int) _lCalib.size() - 1 )
    iNumL1Event = _fileHeader.iNumIndex       - iStartIndex;
  else
    iNumL1Event = _lCalib[iCalib+1].iL1Index  - iStartIndex;
    
  return 0;
}

int IndexFileReader::eventCalibToGlobal(int iCalib, int iEvent, int& iGlobalEvent) const
{
  if (_fdXtcIndex == -1)
    return 1;
    
  if ( iCalib < -1 || iCalib >= (int) _lCalib.size() )
  {
    printf( "IndexFileReader::eventCalibToGlobal(): Invalid Calib# %d (Max # = %d)\n",
      iCalib, _lCalib.size()-1 );
    return 2;
  }   
    
  int iMaxEventNum; 
  numL1EventInCalib( iCalib, iMaxEventNum );
  
  if ( iEvent < 0 || iEvent >= (int) iMaxEventNum )
  {
    printf( "IndexFileReader::eventCalibToGlobal(): Invalid Event# %d in Calib# %d (Max # = %d)\n",
      iEvent, iCalib, iMaxEventNum-1);
    return 3;
  }   

  /*
   * special case: iCalib == -1:
   *   iGlobalEvent = iEvent;      
   */
  
  int iStartIndex = (iCalib >= 0 ? _lCalib[iCalib].iL1Index : 0);
  
  iGlobalEvent = iStartIndex + iEvent;      
  return 0;
}

int IndexFileReader::eventGlobalToCalib(int iGlobalEvent, int& iCalib, int& iEvent) const
{
  if (_fdXtcIndex == -1)
    return 1;

  if ( iGlobalEvent < 0 || iGlobalEvent >= _fileHeader.iNumIndex )
  {
    printf( "IndexFileReader::eventGlobalToCalib(): Invalid global Event# %d (Max # = %d)\n",
      iGlobalEvent, _fileHeader.iNumIndex-1);
    return 2;
  }   

  if (_lCalib.size() == 0 || iGlobalEvent < _lCalib[0].iL1Index)
  {
    iCalib = -1;
    iEvent = iGlobalEvent;
    return 0;
  }    
      
  for (iCalib = 0; iCalib < (int) _lCalib.size(); iCalib++)
  {
    int iEventRemain = iGlobalEvent - _lCalib[iCalib].iL1Index;
    
    int iMaxEventNum; 
    numL1EventInCalib( iCalib, iMaxEventNum );    
    if ( iEventRemain < iMaxEventNum )
    {
      iEvent = iEventRemain;
      return 0;
    }
  }
  
  printf( "IndexFileReader::eventGlobalToCalib(): Cannot find correct Calib Cycle for global Event# %d (Max # = %d)n",
    iGlobalEvent, _fileHeader.iNumIndex-1);
  return 3;
}

int IndexFileReader::eventTimeToGlobal(uint32_t uSeconds, uint32_t uNanoseconds, int& iGlobalEvent, bool& bExactMatch, bool& bOvertime)
{  
  iGlobalEvent  = -1;
  bExactMatch   = false;
  bOvertime     = false;
  if (_fdXtcIndex == -1)
    return 1;  
  
  if ( _fileHeader.iNumIndex == 0 )
    return 0;
  else if ( _fileHeader.iNumIndex == 1 )
  {
    iGlobalEvent = 0;
    return 0;
  }
  
  int iEventLowerBound  = 0;
  int iEventUpperBound  = _fileHeader.iNumIndex-1;
  int iEventTest        = (iEventLowerBound + iEventUpperBound)/2;

  // check special case: lower bound and upper bound
  {
    uint32_t uSecondsTest, uNanosecondsTest;
    
    int iError = time(iEventLowerBound, uSecondsTest, uNanosecondsTest);
    if (iError != 0)
    {
      printf("IndexFileReader::gotoTime(): Cannot get time for global event# %d\n", iEventLowerBound);
      return 2;
    }
    
    int iCompare = compareTime(uSeconds, uNanoseconds, uSecondsTest, uNanosecondsTest);
    if (iCompare <= 0)
    {
      iEventUpperBound = iEventLowerBound; // will pass the for loop below, and output iEventLowerBound as the index            
      if ( iCompare == 0 )
        bExactMatch = true;
    }
    else
    {

      iError = time(iEventUpperBound, uSecondsTest, uNanosecondsTest);
      if (iError != 0)
      {
        printf("IndexFileReader::gotoTime(): Cannot get time for global event# %d\n", iEventUpperBound);
        return 3;
      }
      
      iCompare = compareTime(uSeconds, uNanoseconds, uSecondsTest, uNanosecondsTest);
      if (iCompare > 0)
      {
        bOvertime = true;
        return 4; 
      }
        
      if (iCompare == 0)
      {
        iEventLowerBound = iEventUpperBound; // will pass the for loop below, and output iEventUpperBound as the index
        bExactMatch      = true;
      }
    }
  }
  
  /*
   * Use binary search to find the event with the correct time (the same or later time)
   */
  for (;iEventLowerBound+1 < iEventUpperBound; iEventTest = (iEventLowerBound + iEventUpperBound)/2)
  {
    uint32_t uSecondsTest, uNanosecondsTest;
    
    int iError = time(iEventTest, uSecondsTest, uNanosecondsTest);
    if (iError != 0)
    {
      printf("IndexFileReader::gotoTime(): Cannot get time for global event# %d\n", iEventTest);
      return 4;
    }
    
    int iCompare = compareTime(uSeconds, uNanoseconds, uSecondsTest, uNanosecondsTest);
    if (iCompare > 0)
      iEventLowerBound = iEventTest;
    else if (iCompare < 0)      
      iEventUpperBound = iEventTest;
    else
    {
      iEventUpperBound = iEventTest;
      bExactMatch      = true;
      break;
    }
  }
  
  iGlobalEvent = iEventUpperBound;  
  return 0;
}

int IndexFileReader::eventTimeToCalib(uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime)
{
  iCalib      = -1;
  iEvent      = -1;
  bExactMatch = false;
  bOvertime   = false;
  
  int iGlobalEvent = -1;
  int iError = eventTimeToGlobal( uSeconds, uNanoseconds, iGlobalEvent, bExactMatch, bOvertime );
  if (iError != 0)
    return 1;
    
  iError = eventGlobalToCalib( iGlobalEvent, iCalib, iEvent );
  if (iError != 0)
    return 2;
  
  return 0;
}

int IndexFileReader::detectorList(int& iNumDetector, const ProcInfo*& lDetector) const
{
  iNumDetector = -1;
  if (_fdXtcIndex == -1)
    return 1;
    
  iNumDetector = _fileHeader.iNumDetector;
  lDetector    = &_lDetector[0];

  return 0;
}

int IndexFileReader::srcList(int iDetector, int& iNumSrc, const Src*& lSrc) const
{
  if (_fdXtcIndex == -1)
    return 1;
    
  if (iDetector < 0 || iDetector >= _fileHeader.iNumDetector )
  {
    printf("IndexFileReader::typeList(): Invalid detector id %d (max detId %d)\n",
      iDetector, _fileHeader.iNumDetector-1);
    return 2;
  }
  
  iNumSrc = _lSrcList[iDetector].size();
  lSrc    = &_lSrcList[iDetector][0];

  return 0;
}

int IndexFileReader::typeList(int iDetector, int& iNumType, const TypeId*& lType) const
{
  if (_fdXtcIndex == -1)
    return 1;

  if (iDetector < 0 || iDetector >= _fileHeader.iNumDetector )
  {
    printf("IndexFileReader::typeList(): Invalid detector id %d (max detId %d)\n",
      iDetector, _fileHeader.iNumDetector-1);
    return 2;
  }
  
  iNumType = _lTypeList[iDetector].size();
  lType = &_lTypeList[iDetector][0];

  return 0;
}

int IndexFileReader::numCalibCycle(int& iNumCalib) const
{
  iNumCalib = -1;
  if (_fdXtcIndex == -1)
    return 1;
    
  iNumCalib = _fileHeader.iNumCalib;
  return 0;
}

int IndexFileReader::calibCycleList(const CalibNode*& lCalib) const
{
  if (_fdXtcIndex == -1)
    return 1;
    
  lCalib    = &_lCalib[0];
  return 0;
}

int IndexFileReader::gotoEvent(int iCalib, int iEvent, int64_t& i64offset, int& iGlobalEvent)
{  
  if (_fdXtcIndex == -1)
    return 1;
      
  iGlobalEvent = -1;
  if (iCalib < -1 || iCalib >= (int) _lCalib.size())
  {
    printf( "Invalid Calib# %d\n", iCalib );
    return 2;
  }
     
  if ( iEvent < 0 )
  {
    if (iCalib == -1)
    {
      printf("IndexFileReader::gotoEvent(): cannot goto the beginning of Calib# %d\n",
        iCalib);
      return 3;
    }

    const CalibNode& calibNode = _lCalib[iCalib];
    int iError = gotoL1Node(calibNode.iL1Index);
    if ( iError != 0 )
    {
      printf("IndexFileReader::gotoEvent(): cannot move index to L1 Event# %d\n",
        calibNode.iL1Index);
      return 4;
    }  

    i64offset     = calibNode.i64Offset;
    iGlobalEvent  = calibNode.iL1Index;    
    return 0;
  } 
  
  int iError = eventCalibToGlobal(iCalib, iEvent, iGlobalEvent);
  if (iError != 0)
  {
    printf("IndexFileReader::gotoEvent(): Error converting Calib# %d Event# %d\n",
      iCalib, iEvent);
    return 5;
  }  
    
  iError = gotoL1Node(iGlobalEvent);
  if ( iError != 0 )
  {
    printf("IndexFileReader::gotoEvent(): cannot move index to L1 Event# %d\n",
      iGlobalEvent);
    return 5;
  }        
  
  i64offset = _curL1Node.i64OffsetXtc;
  return 0;
}

int IndexFileReader::gotoEventInXtc(int iCalib, int iEvent, int fdXtc, int& iGlobalEvent)
{         
  int64_t i64Offset = 0;
  int iError = gotoEvent(iCalib, iEvent, i64Offset, iGlobalEvent);
  if (iError != 0)
    return iError;
          
  int64_t i64OffsetSeek = lseek64(fdXtc, i64Offset, SEEK_SET);
  if (i64OffsetSeek != i64Offset)
  {
    printf( "IndexFileReader::gotoEventAndSeek(): seek xtc failed (expected 0x%Lx, get 0x%Lx), error = %s\n",
      i64Offset, i64OffsetSeek, strerror(errno) );
    return 6;
  }    
  
  return 0;    
}


int IndexFileReader::gotoTimeInXtc(uint32_t uSeconds, uint32_t uNanoseconds, int fdXtc, int& iGlobalEvent, bool& bExactMatch, bool& bOvertime)
{  
  iGlobalEvent = -1;

  int iError = eventTimeToGlobal(uSeconds, uNanoseconds, iGlobalEvent, bExactMatch, bOvertime);
  if (iError != 0)
    return 1;
    
  iError = gotoL1Node(iGlobalEvent);
  if (iError != 0)
  {
    printf("IndexFileReader::gotoTime(): cannot move index to L1 event 0x%x (%d)\n",
      iGlobalEvent, iGlobalEvent);
    return 2;
  }        
  
  int64_t i64OffsetSeek = lseek64(fdXtc, _curL1Node.i64OffsetXtc, SEEK_SET);
  if (i64OffsetSeek != _curL1Node.i64OffsetXtc)
  {
    printf( "IndexFileReader::gotoEvent(): seek xtc failed (expected 0x%Lx, get 0x%Lx), error = %s\n",
      _curL1Node.i64OffsetXtc, i64OffsetSeek, strerror(errno) );
    return 3;
  }    
  
  return 0;
}

int IndexFileReader::time(int iGlobalEvent, uint32_t& uSeconds, uint32_t& uNanoseconds)
{
  uSeconds      = 0;
  uNanoseconds  = 0;
  int iError = gotoL1Node(iGlobalEvent);
  if ( iError != 0 )
    return 1;
    
  uSeconds     = _curL1Node.uSeconds;  
  uNanoseconds = _curL1Node.uNanoseconds;
  return 0;
}

int IndexFileReader::fiducial(int iGlobalEvent, uint32_t& uFiducial)
{
  uFiducial = 0x1ffff;
  int iError = gotoL1Node(iGlobalEvent);
  if ( iError != 0 )
    return 1;
    
  uFiducial = _curL1Node.uFiducial;  
  return 0;
}

int IndexFileReader::offset(int iGlobalEvent, int64_t&  i64Offset)
{
  int iError = gotoL1Node(iGlobalEvent);
  if ( iError != 0 )
    return 1;
    
  i64Offset = _curL1Node.i64OffsetXtc;
  return 0;  
}

int IndexFileReader::damage(int iGlobalEvent, Damage& damage)
{
  int iError = gotoL1Node(iGlobalEvent);
  if ( iError != 0 )
    return 1;
    
  damage = _curL1Node.damage;
  return 0;
}

int IndexFileReader::detDmgMask(int iGlobalEvent, uint32_t& uMaskDetDmgs)
{
  int iError = gotoL1Node(iGlobalEvent);
  if ( iError != 0 )
    return 1;
    
  uMaskDetDmgs = _curL1Node.uMaskDetDmgs;
  return 0;
}

int IndexFileReader::detDataMask(int iGlobalEvent, uint32_t& uMaskDetData)
{
  int iError = gotoL1Node(iGlobalEvent);
  if ( iError != 0 )
    return 1;
    
  uMaskDetData = _curL1Node.uMaskDetData;
  return 0;
}

int IndexFileReader::evrEventList(int iGlobalEvent, unsigned int& uNumEvent, const uint8_t*& lEvrEvent)
{
  int iError = gotoL1Node(iGlobalEvent);
  if ( iError != 0 )
    return 1;
          
  uNumEvent = _lCurEvrEvent.size();
  lEvrEvent = &_lCurEvrEvent[0];
  return 0;  
}

int IndexFileReader::gotoL1Node(int iL1Node)
{
  if ( iL1Node == _iCurL1Node )
    return 0;
        
  if (_fdXtcIndex == -1)
    return 1;
    
  if ( iL1Node < 0 || iL1Node >= _fileHeader.iNumIndex )
  {
    printf( "IndexFileReader::gotoL1Node(): Invalid L1 Event# %d\n",
      iL1Node );
    return 2;
  }
      
  int64_t i64OffsetTo   = sizeof(_fileHeader) + sizeof(_curL1Node)* iL1Node;
  int64_t i64OffsetSeek = ::lseek64(_fdXtcIndex, i64OffsetTo, SEEK_SET);
  if ( i64OffsetSeek != i64OffsetTo )
  {
    printf("IndexFileReader::gotoL1Node(): Seek to CalibCycle offset 0x%Lx failed, result = 0x%Lx\n", 
      i64OffsetTo, i64OffsetSeek );
  }
    
  _iCurL1Node = -1;  
  _lCurEvrEvent.clear();
     
  int iRead = ::read(_fdXtcIndex, (void*)&_curL1Node, sizeof(_curL1Node));
  if ( iRead != sizeof(_curL1Node) )
  {
    printf( "IndexFileReader::gotoL1Node(): Read L1 Node [%d] failed. Read %d bytes (expected %d), error = %s\n",
      iL1Node, iRead, sizeof(_curL1Node), strerror(errno) );
    return 3;
  }
  
  /*
   * Generate current Evr events
   */
  if (_curL1Node.uMaskEvrEvents != 0)
  {
    uint32_t uBit = 0x1;
    for (int iBitIndex = 0; iBitIndex < (int)_lEvrEvent.size(); iBitIndex++, uBit <<= 1 )
      if ((_curL1Node.uMaskEvrEvents & uBit) != 0)
        _lCurEvrEvent.push_back(_lEvrEvent[iBitIndex]);
  }
        
  _iCurL1Node = iL1Node;  
  
  return 0;  
}

} // namespace Index
} // namespace Pds
