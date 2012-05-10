#include <fcntl.h>
#include <errno.h>

#include "pdsdata/ana/XtcSlice.hh"

namespace Pds
{  
namespace Ana
{

// local functions
static void*  readSlice(void* arg);
static int    genIndexFromXtcFilename( const std::string& strXtcFilename, std::string& strIndexFilename ); 

XtcSlice::XtcSlice(std::string fname) :
  _base(fname.substr(0,fname.find("-c"))),
  _current(_chunks.begin()),
  _lastdg(0),
  _nextdg(0),
  _fd  (-1),
  _pool(new XtcPool(4,0x4000000)),
  _bclose(false),
  _i64OffsetNext(0)
{
  _chunks.push_back(fname);
}

XtcSlice::~XtcSlice() 
{
  _close(true);
  
  _index.close();      

  delete _pool;
}

bool XtcSlice::add_file(std::string fname) 
{
  if (fname.compare(0,_base.size(),_base)!=0) 
    return false;
  
  for(std::list<std::string>::iterator it=_chunks.begin();
      it!=_chunks.end(); it++)
    if (fname < (*it)) {
      _chunks.insert(it,fname);
      return true;
    }
  _chunks.push_back(fname);
  return true;
}

void XtcSlice::init()
{
  _current = _chunks.begin();
  if (!_open())
    exit(1);    
}

Result XtcSlice::next(Pds::Dgram*& dg, int64_t* pi64OffsetCur)
{
  if (pi64OffsetCur != NULL)
    *pi64OffsetCur = _i64OffsetNext;
  dg = _nextdg;
  return _next();    
}

Result XtcSlice::skip()
{
  Pds::Dgram* dg;
  return next(dg);
}

bool XtcSlice::_open(int64_t i64Offset)
{  
  if (_fd != -1)
    _close(true);
      
  const char* fname = _current->c_str();
  _fd = ::open(fname,O_LARGEFILE,O_RDONLY);
  if (_fd == -1) {
    char buff[256];
    sprintf(buff,"Error opening file %s\n",fname);
    perror(buff);
    return false;
  }
  
  if ( i64Offset != 0 )
  {
    int64_t i64OffsetSeek = lseek64(_fd, i64Offset, SEEK_SET);
    if (i64OffsetSeek != i64Offset)
    {
      printf( "XtcSlice::_open(): seek failed (expected 0x%Lx, get 0x%Lx), error = %s\n",
        (long long) i64Offset, (long long) i64OffsetSeek, strerror(errno) );
      return false;
    }        
  }
  
  _i64OffsetNext = i64Offset;

  pthread_create(&_threadID, NULL, readSlice, this);

  /*
   * Here we don't release _lastdg, because it is being used by the caller
   */
  _nextdg = _pool->pop(0);
  //_lastdg = 0;

  return true;
}

void XtcSlice::_close(bool bForceWait) 
{
  if (_live || bForceWait) {
    _bclose=true;
    _pool->unblock();
    
    pthread_join(_threadID,NULL);
    
    delete[] _lastdg; _lastdg = NULL;    
    delete _nextdg; _nextdg = NULL;
    
    delete _pool;
    _pool = new XtcPool(4,0x4000000);
    _bclose =false;
  }
  if (_fd != -1)
    ::close(_fd); 
  _fd = -1;    
  
  _i64OffsetNext = 0;
}

Result XtcSlice::_openChunk(int iChunk, uint64_t i64Offset)
{
  char sChunk[5];
  sprintf(sChunk, "-c%02d", iChunk);
  for (std::list<std::string>::iterator 
    itChunk =  _chunks.begin();
    itChunk != _chunks.end();
    ++itChunk)
  {
    size_t uFind = itChunk->find(sChunk);
    if (uFind == std::string::npos)
      continue;
      
    //if (_fd != -1)
    _close(true);      
    _current = itChunk;
    return ( _open(i64Offset) ? OK : Error );
  }      
    
  /*
   * Special case: The xtc filename doesn't follow the "eXX-rXXXX-sXX-cXX.xtc" format, 
       such as "1.xtc", "test1.xtc", etc. In this case we only have ONE xtc file
       in this slice.
   */
  if (iChunk == 0 && _chunks.size() == 1)
  {
    _close(true);      
    _current = _chunks.begin();
    return ( _open(i64Offset) ? OK : Error );    
  }
  
  printf("XtcSlice::_openChunk(): Cannot find chunk %d\n", iChunk);
  return Error;
}

Result XtcSlice::_next()
{
  bool endRun = (_nextdg->seq.service()==Pds::TransitionId::EndRun);
  
  _i64OffsetNext += sizeof(*_nextdg) + _nextdg->xtc.extent - sizeof(_nextdg->xtc);

  Pds::Dgram* dg = _nextdg;
  _nextdg = _pool->pop(_lastdg);
  _lastdg = dg;
    
  if (!_nextdg) {
    if (_live) {
      if (endRun) {
        _close();
        return End;
      }
      else {  // seek a new chunk else retry
        while(_nextdg==0) {
          int chunkPosition = _current->find("-c")+2;
          int chunkIndex = strtoul(_current->c_str()+chunkPosition, NULL, 10)+1;
          char chunkBuff[4]; sprintf(chunkBuff,"%02d",chunkIndex);
          std::string fname = _current->substr(0,chunkPosition) + std::string(chunkBuff) +
            _current->substr(chunkPosition+2);

          //  Can't use stat here since it fails on files > 2GB
          //struct stat _stat;
          //if (stat(fname.c_str(),&_stat)==0) {
          int fd = ::open(fname.c_str(),O_LARGEFILE,O_RDONLY);
          if (fd != -1) {
            ::close(fd);
            _chunks.push_back(fname);
            _current = _chunks.begin();
            while( *_current != fname )
              ++_current;
            _close();
            if (!_open())
              return Error;
            else
              return OK;
          }
          else {
            _nextdg = _pool->pop(_lastdg);
            _lastdg = 0;
          }
        }
      }
    }
    else {
      std::string sCurrent = *_current;
      _close();
      if (++_current == _chunks.end()) 
      {
        if (!endRun)
          printf("Unexpected eof in %s\n",sCurrent.c_str());        
        return End;
      }
      if (!_open())
        return Error;
        
      return OK;
    }
  }

  return OK;
}

bool XtcSlice::read()
{
  return !_bclose && _pool->push(_fd);
}

Result XtcSlice::_loadIndex()
{
  if ( _index.isValid() )
    return OK;
  
  std::string strIndexFilename;
  int iError = genIndexFromXtcFilename(*_current, strIndexFilename);
  if ( iError != 0 )
    return Error;
  
  _index.open(strIndexFilename.c_str());  
  if ( _index.isValid() )
    return OK;  
    
  printf( "XtcSlice::_loadIndex(): Error loading index file %s for %s\n", strIndexFilename.c_str(), _current->c_str() );    
  return Error;
}

Result XtcSlice::numCalib(int& iNumCalib)
{
  iNumCalib = -1;
  
  _loadIndex();
  if ( !_index.isValid() )
  {
    printf( "XtcSlice::numCalib(): No index file found for %s.\n", 
      _current->c_str() );
    return Error;
  }
    
  int iError = _index.numCalibCycle(iNumCalib);    
  if ( iError != 0 )
  {
    printf( "XtcSlice::numCalib(): Query Calib# failed\n" );
    return Error;
  }
  
  return OK;
}

Result XtcSlice::numEventInCalib(int calib, int& iNumEvents)
{
  iNumEvents = -1;
  
  _loadIndex();
  if ( !_index.isValid() )
  {
    printf( "XtcSlice::numEventInCalib(): No index file found for %s.\n", 
      _current->c_str() );
    return Error;
  }
  
  int iNumCalib = -1;  
  int iError    = _index.numCalibCycle(iNumCalib);    
  if ( iError != 0 )
  {
    printf( "XtcSlice::numEventInCalib(): Query Calib# failed\n" );
    return Error;
  }
  
  if (calib < 1 || calib > iNumCalib)
  {
    printf( "XtcSlice::numEventInCalib(): Invalid Calib# %d. Max # = %d\n", calib, iNumCalib );
    return Error;
  }
    
  // adjust indexes: from 1-based index to 0-based index
  int iCalibAdj = calib - 1;
    
  iError = _index.numL1EventInCalib(iCalibAdj, iNumEvents);
  if ( iError != 0 )
  {
    printf( "XtcSlice::numEventInCalib(): Query Calib Event# failed\n" );
    return Error;
  }
  
  return OK;
}

Result XtcSlice::numTotalEvent(int& iNumTotalEvents)
{
  iNumTotalEvents = -1;
  
  _loadIndex();
  if ( !_index.isValid() )
  {
    printf( "XtcSlice::numTotalEvent(): No index file found for %s.\n", 
      _current->c_str() );
    return Error;
  }
  
  int iError    = _index.numL1Event(iNumTotalEvents);    
  if ( iError != 0 )
  {
    printf( "XtcSlice::numTotalEvent(): Query L1 Event# failed\n" );
    return Error;
  }
  
  return OK;  
}

Result XtcSlice::getTime(int calib, int event, uint32_t& uSeconds, uint32_t& uNanoseconds)
{
  uSeconds      = 0;
  uNanoseconds  = 0;
  
  _loadIndex();
  if ( !_index.isValid() )
  {
    printf( "XtcSlice::getTime(): No index file found for %s. Cannot do the jump\n", 
      _current->c_str() );
    return Error;
  }
    
  // adjust indexes: from 1-based index to 0-based index
  int iCalibAdj = calib - 1;
  int iEventAdj = event - 1;
    
  int iGlobalEvent = -1;
  int iError = _index.eventCalibToGlobal(iCalibAdj, iEventAdj, iGlobalEvent);
  if ( iError != 0 )
    return Error;
    
  iError = _index.time(iGlobalEvent, uSeconds, uNanoseconds);
  if ( iError != 0 )
    return Error;    
  
  return OK;
}

Result XtcSlice::getTimeGlobal(int iSliceEvent, uint32_t& uSeconds, uint32_t& uNanoseconds)
{
  uSeconds      = 0;
  uNanoseconds  = 0;
  
  _loadIndex();
  if ( !_index.isValid() )
  {
    printf( "XtcSlice::getTimeGlobal(): No index file found for %s. Cannot do the jump\n", 
      _current->c_str() );
    return Error;
  }
    
  // adjust indexes: from 1-based index to 0-based index
  int iSliceEventAdj = iSliceEvent - 1;    
  int iError = _index.time(iSliceEventAdj, uSeconds, uNanoseconds);
  if ( iError != 0 )
    return Error;    
  
  return OK;
}

Result XtcSlice::jump(int calib, int jump, int& eventNum, bool bOverJump)
{
  _loadIndex();
  if ( !_index.isValid() )
  {
    printf( "XtcSlice::jump(): No index file found for %s. Cannot do the jump\n", 
      _current->c_str() );
    return Error;
  }

  int iNumCalib = 0;
  int iError = numCalib(iNumCalib);
  if (iError != 0)
  {
    printf("XtcSlice::jump(): Cannot get Calib# \n");
    return Error;
  }                
  
  if ( calib <= 0 || calib > iNumCalib )
  {
    printf("XtcSlice::jump(): Invalid Calib# %d (Max # = %d)\n", calib, iNumCalib);
    return Error;
  }
  
  // adjust indexes: from 1-based index to 0-based index
  int iCalibAdj = calib - 1;
    
  int iNumL1EventInCalib;
  iError = _index.numL1EventInCalib(iCalibAdj, iNumL1EventInCalib);
  if ( iError != 0 )
  {
    printf( "XtcSlice::jump(): Cannot get Event# in Calib# %d\n", calib );
    return Error;
  }
    
  int iJumpAdj = jump - 1;             
  if ( iJumpAdj >= iNumL1EventInCalib )
  {
    if (bOverJump && iJumpAdj == iNumL1EventInCalib)
      --iJumpAdj;
    else
    {
      printf( "XtcSlice::jump(): Invalid Event# %d in calib Calib# %d (max Event# = %d)\n",
        jump, calib, iNumL1EventInCalib);
      return Error;    
    }
  }
  else
    bOverJump = false;
    
  int       iChunk        = -1;
  int       iEventNumAdj  = 0;
  int64_t   i64Offset     = 0;
  iError = _index.gotoEvent(iCalibAdj, iJumpAdj, iChunk, i64Offset, iEventNumAdj);
  if (iError != 0)
  {
    printf("Failed to jump to Calib# %d Event# %d in %s\n", 
      calib, jump, _current->c_str());
    return Error;
  }
  
  //!!debug
  //printf("Convert Calib# %d Event# %d to global Event# %d in Chunk %d Offset 0x%Lx\n",
  //  calib, jump, iEventNumAdj, iChunk, i64Offset);
  
  // adjust indexes: from 0-based index to 1-based index
  eventNum = iEventNumAdj+1;  
  _openChunk(iChunk, i64Offset);
  
  if (!bOverJump)
    printf("Jumped to Calib# %d Event# %d in %s\n", calib, jump, _current->c_str());
  else
  {
    ++eventNum;
    skip();
    printf("Jumped to Calib# %d Event# %d (Overjump) in %s\n", calib, jump, _current->c_str());
  }
  
  return OK;
}

Result XtcSlice::findTime(uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime)
{
  iCalib = -1;
  iEvent = -1;
  
  _loadIndex();
  if ( !_index.isValid() )
  {
    printf( "XtcSlice::findTime(): No index file found for %s.\n", 
      _current->c_str() );
    return Error;
  }
  
  int iCalibAdj = -1, iEventAdj = -1;
  int iError = 
    _index.eventTimeToCalib(uSeconds, uNanoseconds, iCalibAdj, iEventAdj, bExactMatch, bOvertime);
  if ( iError != 0 )
    return Error;

  iCalib = iCalibAdj + 1;
  iEvent = iEventAdj + 1;    
    
  return OK;
}

Result XtcSlice::findTimeGlobal(uint32_t uSeconds, uint32_t uNanoseconds, int& iSliceEvent, bool& bExactMatch, bool& bOvertime)
{
  iSliceEvent = -1;
  
  _loadIndex();
  if ( !_index.isValid() )
  {
    printf( "XtcSlice::findTimeGlobal(): No index file found for %s.\n", 
      _current->c_str() );
    return Error;
  }
  
  int iSliceEventAdj = -1;
  int iError = 
    _index.eventTimeToGlobal(uSeconds, uNanoseconds, iSliceEventAdj, bExactMatch, bOvertime);
  if ( iError != 0 )
    return Error;

  iSliceEvent = iSliceEventAdj + 1;    
    
  return OK;
}

Result XtcSlice::findNextFiducial(uint32_t uFiducial, int iFromEvent, int& iEvent)
{
  iEvent = -1;
  
  _loadIndex();
  if ( !_index.isValid() )
  {
    printf( "XtcSlice::findNextFiducial(): No index file found for %s. Cannot do the jump\n", 
      _current->c_str() );
    return Error;
  }
  
  int iFromEventAdj = iFromEvent - 1;
  int iEventAdj     = -1;
  int iError = 
    _index.eventNextFiducialToGlobal(uFiducial, iFromEventAdj, iEventAdj);
  if ( iError != 0 )
    return Error;

  iEvent = iEventAdj + 1;    
    
  return OK;
}

void* readSlice(void* arg)
{
  XtcSlice* s = (XtcSlice*)arg;
  while(s->read()) ;  
  return NULL;
}

static int genIndexFromXtcFilename( const std::string& strXtcFilename, std::string& strIndexFilename )
{ 
  strIndexFilename.clear();
  size_t iFindPos = strXtcFilename.rfind(".xtc");
  
  if (iFindPos == std::string::npos )
    return 1;
    
  strIndexFilename = strXtcFilename.substr(0, iFindPos) + ".xtc.idx";  
    
  struct ::stat64 statFile;
  int iError = ::stat64( strIndexFilename.c_str(), &statFile );
  if ( iError != 0 )
  {
    size_t iFindDir = strXtcFilename.rfind("/");
    if (iFindDir == std::string::npos )
      strIndexFilename = "index/" + strXtcFilename + ".idx";
    else
      strIndexFilename = strXtcFilename.substr(0, iFindDir+1) + "index/" + strXtcFilename.substr(iFindDir+1) + ".idx";    
    
    iError = ::stat64( strIndexFilename.c_str(), &statFile );    
    if ( iError != 0 )
    {
      strIndexFilename.clear();
      return 2;
    }    
  }
  
  //printf( "Using %s as the index file for analyzing %s\n", 
  //  strIndexFilename.c_str(), strXtcFilename.c_str() );
  
  return 0;  
}

} // namespace Ana
} // namespace Pds

