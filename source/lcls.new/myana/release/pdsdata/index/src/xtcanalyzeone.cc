#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/BldInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/index/IndexFileReader.hh"
#include "pdsdata/index/IndexChunkReader.hh"

using std::string;
using namespace Pds;

class XtcIterWithOffset: public XtcIterator
{
public:
  enum
  { Stop, Continue };
  
  XtcIterWithOffset(Xtc * xtc, unsigned depth, int64_t i64Offset) :
    XtcIterator(xtc), _depth(depth), _i64Offset(i64Offset), _iNumXtc(0)
  {}

  int  process(Xtc * xtc);
  
protected:
  unsigned            _depth;
  int64_t             _i64Offset;
  int                 _iNumXtc;
};

class XtcIterConfig: public XtcIterWithOffset
{
public:  
  XtcIterConfig(Xtc * xtc, unsigned depth, int64_t i64Offset) :
    XtcIterWithOffset(xtc, depth, i64Offset)
  {}

  int  process(Xtc * xtc);  
};

class XtcIterL1Accept: public XtcIterWithOffset
{
public:
  XtcIterL1Accept(Xtc * xtc, unsigned depth, int64_t i64Offset):
    XtcIterWithOffset(xtc, depth, i64Offset)
  {}

  int  process(Xtc * xtc);
};

void usage(char *progname)
{
  printf( 
    "Usage:  %s  [-f <xtc filename>] [-i <index>] [-o <offset>] "
    "[-n <output L1 event#>] [-j <begin L1 event#>] [-y <begin calib cycle#>] [-t <time>]" 
    "[-u <fiducial>[,<event>]] [-h]\n"
    "  Options:\n"
    "    -h                       Show usage.\n"
    "    -f <xtc filename>        Set xtc filename\n"
    "    -i <index filename>      Set index filename\n"
    "    -n <output L1 event#>    Set L1 event# for ouput\n"
    "    -o <offset>              Start analysis from offset\n"
    "    -j <begin L1 event#>     Set begin L1 event#\n"
    "    -y <begin calib cycle#>  Set begin calib cycle#\n"
    "    -t <time>                Go to the event at <time>\n"
    "    -u <fiducial>[,<event>]  Go to the event with fiducial <fiducial>, searching from <event>\n"
    ,    
    progname
  );
}

int xtcAnalyze( const char* sXtcFilename, const char* sIndexFilename, 
  int iBeginL1Event, int iNumL1Event, int iBeginCalib, 
  int64_t i64OffsetStart, char* sTime, uint32_t uFiducialSearch, int iFidFromEvent );

int main(int argc, char *argv[])
{
  char*         sXtcFilename    = NULL;
  char*         sIndexFilename  = NULL;
  int           iBeginL1Event   = 0;
  int           iNumL1Event     = 0;
  int           iBeginCalib     = 0;
  int64_t       i64OffsetStart  = 0;
  char*         sTime           = NULL;
  uint32_t      uFiducialSearch = -1;
  int           iFidFromEvent   = 0;

  int c;
  while ((c = getopt(argc, argv, "hf:i:n:o:j:y:t:u:")) != -1)
  {
    switch (c)
    {
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'f':
      sXtcFilename  = optarg;
      break;
    case 'i':
      sIndexFilename= optarg;
      break;
    case 'j':
      iBeginL1Event = strtol(optarg, NULL, 0);
      break;
    case 'y':
      iBeginCalib   = strtol(optarg, NULL, 0);
      break;
    case 'n':
      iNumL1Event   = strtol(optarg, NULL, 0);
      break;
    case 'o':
      i64OffsetStart= strtoll(optarg, NULL, 0);
      break;
    case 't':
      sTime         = optarg;
      break;
    case 'u':
      uFiducialSearch = strtoul(optarg, NULL, 0);
      {
      char* sNextParam = strchr(optarg,',');
      if (sNextParam != NULL)
        iFidFromEvent = strtoul(sNextParam+1, NULL, 0);
      }      
      break;
    default:
      printf( "Unknown option: -%c", c );
    }
  }
  
  if (!sXtcFilename)
  {
    usage(argv[0]);
    exit(2);
  }
  
  if ( iNumL1Event < 0 )
  {
    printf( "main(): Output L1 Event # %d < 0\n", iNumL1Event );
    return 0;
  }
    
  return xtcAnalyze( sXtcFilename, sIndexFilename, iBeginL1Event, iNumL1Event, iBeginCalib, i64OffsetStart, 
    sTime, uFiducialSearch, iFidFromEvent);
}

int printIndexSummary(const Index::IndexFileReader& indexFileReader)
{
  int iNumCalib;
  indexFileReader.numCalibCycle(iNumCalib);
  
  const Index::CalibNode* lCalib;
  indexFileReader.calibCycleList(lCalib);
  
  printf( "Num of Calib Cycle: %d\n", iNumCalib );  
  for ( int iCalib = 0; iCalib < iNumCalib; iCalib++ )
  {
    const Index::CalibNode& calibNode = lCalib[iCalib];
    
    char sTimeBuff[128];
    time_t t = calibNode.uSeconds;
    strftime(sTimeBuff,128,"%Z %a %F %T",localtime(&t));  
        
    printf( "Calib %d Off 0x%Lx L1 %d %s.%03u\n", iCalib, (long long) calibNode.i64Offset, calibNode.iL1Index,
      sTimeBuff, (int)(calibNode.uNanoseconds/1e6));
  }          
  
  int             iNumDetector;
  const ProcInfo* lDetector;
  indexFileReader.detectorList(iNumDetector, lDetector);
  printf( "Num of Detector: %d\n", iNumDetector );  
  for ( int iDetector = 0; iDetector < iNumDetector; iDetector++ )
  {
    const ProcInfo& info = lDetector[iDetector];
    printf( "Segment %d ip 0x%x pid 0x%x\n", 
      iDetector, info.ipAddr(), info.processId());
      
    int         iNumSrc = 0;
    const Src*  lSrc    = NULL;
    indexFileReader.srcList(iDetector, iNumSrc, lSrc);
    
    const TypeId* lType = NULL;
    indexFileReader.typeList(iDetector, iNumSrc, lType);
    
    for (int iSrc=0; iSrc<iNumSrc; iSrc++)
    {
      const Src& src = (Src&) lSrc[iSrc];
      if (src.level() == Level::Source)
      {        
        const DetInfo& info = (const DetInfo&) src;
        printf("  src %s,%d %s,%d ",
         DetInfo::name(info.detector()), info.detId(),
         DetInfo::name(info.device()), info.devId());           
      }
      else if ( src.level() == Level::Reporter )
      {
        const BldInfo& info = (const BldInfo&) src;
        printf("  bldType %s ", BldInfo::name(info));          
      }
        
      const TypeId& typeId = lType[iSrc];
      printf("contains %s V%d\n",
       TypeId::name(typeId.id()), typeId.version()
       );                
    }
  }        
  
  return 0;
}

int printEvent(Index::IndexFileReader& indexFileReader, int iEvent)
{
  int uNumL1Event = 0;
  int iError = indexFileReader.numL1Event(uNumL1Event);
  if ( iError != 0 ) return 1;

  if (iEvent < 0 || iEvent >= (int) uNumL1Event)
  {
    printf("printEvent(): Invalid event %d\n", iEvent);
    return 2;
  }

  uint32_t uSeconds, uNanoSeconds;
  indexFileReader.time(iEvent, uSeconds, uNanoSeconds);
  
  uint32_t  uFiducial;
  indexFileReader.fiducial(iEvent, uFiducial);
      
  Damage damage(0);
  indexFileReader.damage(iEvent, damage);
  
  uint32_t uMaskDetDmgs = 0;
  indexFileReader.detDmgMask(iEvent, uMaskDetDmgs);

  uint32_t uMaskDetData = 0;
  indexFileReader.detDataMask(iEvent, uMaskDetData);
  
  unsigned int uNumEvent = 0;
  const uint8_t* lEvrEvent;
  indexFileReader.evrEventList(iEvent, uNumEvent, lEvrEvent);
  
  char sTimeBuff[128];
  time_t t = uSeconds;
  strftime(sTimeBuff,128,"%T",localtime(&t));  
    
  printf( "[%d] %s.%03u Fid 0x%05x Dmg 0x%x ", iEvent, 
    sTimeBuff, (int)(uNanoSeconds/1e6), uFiducial, damage.value() );
    
  printf( "DetDmg 0x%x DetData 0x%x ", uMaskDetDmgs, uMaskDetData );
  
  /*
   * print events     
   */
  printf( "Evn%d ", uNumEvent );
  if ( uNumEvent != 0 )
    for ( unsigned int uEvent = 0; uEvent < uNumEvent; uEvent++ )
      printf( "[%d] ", (int) lEvrEvent[uEvent] );
        
  printf( "\n" );
  
  return 0;  
}

int gotoEvent(const char* sIndexFilename, int iBeginL1Event, int iBeginCalib, 
  char* sTime, uint32_t uFiducialSearch, int iFidFromEvent, int fdXtc, 
  int& iGlobalEvent, int& iCalib, int& iCalibEvent)
{
  iGlobalEvent  = -1;
  iCalib        = -1;
  iCalibEvent   = -1;
    
  Index::IndexFileReader indexFileReader;
  int iError = indexFileReader.open( sIndexFilename );
  if ( iError != 0 )
    return 1;  
  
  uint32_t uSeconds = 0, uNanoseconds = 0;
  
  if ( sTime == NULL )
  {
    int iNumEvent = -1;
    indexFileReader.numL1EventInCalib(iBeginCalib, iNumEvent);
    
    printf("Using index file %s to jump to Calib# %d Event# %d (Max Event# in Calib: %d)\n", 
      sIndexFilename, iBeginCalib, iBeginL1Event, iNumEvent-1 );
      
    //int iGlobalEvent = -1, iCalib = -1, iEvent = -1;
    //indexFileReader.eventLocalToGlobal(iBeginCalib, iBeginL1Event, iGlobalEvent);    
    //indexFileReader.eventGlobalToLocal(iGlobalEvent, iCalib, iEvent);    
    //printf( "Global event# %d -> Local Calib# %d Event# %d\n", iGlobalEvent, iCalib, iEvent );
  }
  else
  {
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
    
    //printf( "time %d %d %d %d %d %d %d %d %d\n",
    //  tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon, 
    //  tm.tm_year, tm.tm_wday, tm.tm_yday, tm.tm_isdst );
      
    uSeconds = mktime(&tm);
    
    char* pDot = strchr(sTime, '.');
    if ( pDot != NULL)
    {
      double fNanoseconds = strtod(pDot, NULL);
      uNanoseconds = (uint32_t)(fNanoseconds * 1e9 + 0.5);
    }
    
    char sTimeBuff[128];
    strftime(sTimeBuff, 128, "%Z %F %T", &tm);
    printf("Using index file %s to jump to time %s.%03u%s (seconds 0x%x nanosecs 0x%x)\n",
      sIndexFilename, sTimeBuff, (int)(uNanoseconds/1e6), 
      ( ( pDST != NULL ) ? " (adjusted by DST)" : "" ),
      uSeconds, uNanoseconds);
  }          
  
  printIndexSummary(indexFileReader);

  if ( iBeginL1Event != 0 || iBeginCalib != 0 )
  {
    //{//!!debug 
    //  Pds::Index::IndexChunkReader indexChunk;
    //  indexChunk.open(sIndexFilename); 
    //  
    //  int     iChunk = -1;
    //  int64_t i64Offset = -1;
    //  iError = 
    //    indexChunk.gotoEvent(iBeginCalib, iBeginL1Event, iChunk, i64Offset, iGlobalEvent);
    //  
    //  printf("IndexChunk convert Calib# %d Event# %d to global Event# %d in Chunk %d Offset 0x%Lx\n",
    //    iBeginCalib, iBeginL1Event, iGlobalEvent, iChunk, i64Offset);      
    //}

    iError = 
      indexFileReader.gotoEventInXtc(iBeginCalib, iBeginL1Event, fdXtc, iGlobalEvent);
    if ( iError != 0 )
    {
      printf("Failed to jump to Calib# %d Event# %d\n", iBeginCalib, iBeginL1Event);
      return 5;
    }
  }
  else if ( sTime != NULL )
  {        
    bool bExactMatch = false;
    bool bOvertime   = false;
    
    {//!!debug 
      Pds::Index::IndexChunkReader indexChunk;
      indexChunk.open(sIndexFilename); 
      iError = indexChunk.eventTimeToGlobal(uSeconds, uNanoseconds, iGlobalEvent, bExactMatch, bOvertime);      
        
      int     iChunk      = -1;
      int     iEventChunk = -1;
      iError = indexChunk.eventGlobalToChunk(iGlobalEvent, iChunk, iEventChunk);
      
      int64_t i64Offset   = -1;
      iError = indexChunk.offset(iGlobalEvent, i64Offset);
        
      printf("IndexChunk convert time %s to global Event# %d Match %s Overtime %s in Chunk %d Offset 0x%Lx\n",
        sTime, iGlobalEvent, (bExactMatch?"Y":"n"), (bOvertime?"Y":"n"), iChunk, (long long) i64Offset);      
        
    }
    
    iError = 
      indexFileReader.gotoTimeInXtc( uSeconds, uNanoseconds, fdXtc, iGlobalEvent, bExactMatch, bOvertime );
    if ( iError != 0 )
    {
      if (bOvertime)
        printf("Time %s is later than the last event\n", sTime);
      else
        printf("Failed to jump to time %s\n", sTime);
      
      return 7;
    }    
  }  
  else if ( uFiducialSearch != (uint32_t) -1 )
  {
    printf("Searching for fiducial 0x%x  eventFrom %d\n", uFiducialSearch, iFidFromEvent);
    iError = 
      indexFileReader.gotoNextFiducialInXtc( uFiducialSearch, iFidFromEvent, fdXtc, iGlobalEvent);
    if ( iError != 0 )
    {
      printf("Failed to jump to next fiducial %x from event %d\n", uFiducialSearch, iFidFromEvent);
      return 6;
    }    
  }

  indexFileReader.eventGlobalToCalib(iGlobalEvent, iCalib, iCalibEvent);    
  printf( "Going to Calib# %d Event# %d (global event# %d)\n", iCalib, iCalibEvent, iGlobalEvent );
  
  printEvent(indexFileReader, iGlobalEvent);  
  
  return 0;
}

int genIndexFromXtcFilename( const string& strXtcFilename, string& strIndexFilename )
{  
  size_t iFindPos = strXtcFilename.rfind(".xtc");
  
  if (iFindPos == string::npos )
    return 1;
    
  strIndexFilename = strXtcFilename.substr(0, iFindPos) + ".xtc.idx";  
  
  struct ::stat64 statFile;
  int iError = ::stat64( strIndexFilename.c_str(), &statFile );
  if ( iError != 0 )
  {
    strIndexFilename.clear();
    return 2;
  }
  
  printf( "Using %s as the index file for analyzing %s\n", 
    strIndexFilename.c_str(), strXtcFilename.c_str() );
  
  return 0;  
}

int updateEvr(const Xtc& xtc)
{
  // assume xtc.contains.id() == TypeId::Id_EvrData
  
  if ( xtc.contains.version() != 3 )
  {
    printf( "UnExtorted Evr Data Ver %d\n", xtc.contains.version() );
    return 1;
  }
  
  const EvrData::DataV3& evrData = * reinterpret_cast<const EvrData::DataV3*>(xtc.payload());
  
  for ( unsigned int uEvent = 0; uEvent < evrData.numFifoEvents(); uEvent++ )
  {
    const EvrData::DataV3::FIFOEvent& fifoEvent = 
      evrData.fifoEvent(uEvent);
      
    printf( "[%u] ", fifoEvent.EventCode);      
  }  
  
  return 0;
}

int xtcAnalyze( const char* sXtcFilename, const char* sIndexFilename, 
  int iBeginL1Event, int iNumL1Event, int iBeginCalib, 
  int64_t i64OffsetStart, char* sTime, uint32_t uFiducialSearch, int iFidFromEvent )
{    
  int fdXtc = open(sXtcFilename, O_RDONLY | O_LARGEFILE);
  if (fdXtc < 0)
  {
    printf("Unable to open xtc file %s\n", sXtcFilename);
    return 1;
  }
  
  int iGlobalEvent= 0;
  int iCalib      = -1;
  int iCalibEvent = 0;
  int iEndL1Event = -1;
  if (iBeginL1Event != 0 || iBeginCalib != 0 || sTime != NULL || uFiducialSearch != (uint32_t) -1)
  {
    string strIndexFilename;
    
    if ( sIndexFilename == NULL )
    {            
      genIndexFromXtcFilename( sXtcFilename, strIndexFilename );
      if ( strIndexFilename.size() == 0 )
      {
        printf( "xtcAnalyze(): Cannot do fast seeking without using index file" );      
        return 1;
      }
      sIndexFilename = strIndexFilename.c_str();
    }
          
    int iGlobalEventUpdate = -1;
    int iError = gotoEvent(sIndexFilename, iBeginL1Event, iBeginCalib, sTime, uFiducialSearch, iFidFromEvent,
      fdXtc, iGlobalEventUpdate, iCalib, iCalibEvent);
    if ( iError == 0 )
      iGlobalEvent = iGlobalEventUpdate;      
  }  
  
  if ( iNumL1Event > 0 )
    iEndL1Event = iGlobalEvent + iNumL1Event;      
  
  XtcFileIterator   iterFile(fdXtc, 0x2000000); // largest L1 data: 32 MB
  
  if ( i64OffsetStart != 0 )
  {
    int64_t i64OffsetSeek = lseek64(fdXtc, i64OffsetStart, SEEK_SET);
    if ( i64OffsetSeek != i64OffsetStart )
    {
      printf("Seek to offset 0x%Lx failed, result = 0x%Lx\n", 
        (long long) i64OffsetStart, (long long) i64OffsetSeek );
    }
    else
      printf("Seek to offset 0x%Lx\n", (long long) i64OffsetStart );      
  }  
    
  Dgram *dg;
  int64_t i64Offset = lseek64(fdXtc, 0, SEEK_CUR);
  while ((dg = iterFile.next()))
  {             
    bool bEndLoop = false;
    
    char sTimeBuff[128], sDateTimeBuff[128];
    time_t t = dg->seq.clock().seconds();
    strftime(sTimeBuff,128,"%T",localtime(&t));
    strftime(sDateTimeBuff,128,"%Z %a %F %T",localtime(&t));
    
    switch ( dg->seq.service() )
    {
    case TransitionId::Configure:
    {
      printf( "\n# %s ctl 0x%x vec %d fid 0x%x %s.%03u "
       "offset 0x%Lx env 0x%x damage 0x%x extent 0x%x\n",
       TransitionId::name(dg->seq.service()), dg->seq.stamp().control(),       
       dg->seq.stamp().vector(), dg->seq.stamp().fiducials(), 
       sDateTimeBuff, (int) (dg->seq.clock().nanoseconds() / 1e6),
       (long long) i64Offset, dg->env.value(), dg->xtc.damage.value(), dg->xtc.extent);    
      
      // Go through the config data and create the cfgSegList object
      XtcIterConfig iterConfig(&(dg->xtc), 0, i64Offset + sizeof(*dg) );
      iterConfig.iterate();
      
      break;        
    }    
    case TransitionId::L1Accept:
    {
      printf( "\n# %s #%d ctl 0x%x vec %d fid 0x%x %s.%03u "
       "offset 0x%Lx env 0x%x damage 0x%x extent 0x%x calib %d event %d\n",
       TransitionId::name(dg->seq.service()), iGlobalEvent, dg->seq.stamp().control(),
       dg->seq.stamp().vector(), dg->seq.stamp().fiducials(), 
       sTimeBuff, (int) (dg->seq.clock().nanoseconds() / 1e6),
       (long long) i64Offset, dg->env.value(), dg->xtc.damage.value(), dg->xtc.extent,
       iCalib, iCalibEvent);    
      
      XtcIterL1Accept iterL1Accept(&(dg->xtc), 0, i64Offset + sizeof(*dg) );
      iterL1Accept.iterate();          
      ++iGlobalEvent;
      ++iCalibEvent;
      
      if ( iEndL1Event >= 0 && iGlobalEvent >= iEndL1Event )
        bEndLoop = true;
      break;        
    }   
    default:
      if (dg->seq.service() == TransitionId::BeginCalibCycle)
      {
        ++iCalib;
        iCalibEvent = 0;
      }
        
      printf( "\n# %s ctl 0x%x vec %d fid 0x%x %s.%03u "
       "offset 0x%Lx env 0x%x damage 0x%x extent 0x%x\n",
       TransitionId::name(dg->seq.service()), dg->seq.stamp().control(),
       dg->seq.stamp().vector(), dg->seq.stamp().fiducials(), 
       sDateTimeBuff, (int) (dg->seq.clock().nanoseconds() / 1e6),
       (long long) i64Offset, dg->env.value(), dg->xtc.damage.value(), dg->xtc.extent);           
       
      XtcIterWithOffset iterDefault(&(dg->xtc), 0, i64Offset + sizeof(*dg) );
      iterDefault.iterate();                   
      break;
    } // switch ( dg->seq.service() )
    
    if ( bEndLoop ) break;
      
    i64Offset = lseek64(fdXtc, 0, SEEK_CUR); // get the file offset for the next iteration
  }

  ::close(fdXtc);
  return 0;
}

int XtcIterWithOffset::process(Xtc * xtc)
{
  Level::Type     level             = xtc->src.level();
  int64_t         i64OffsetPayload  = _i64Offset + sizeof(Xtc);
  const DetInfo&  info              = (const DetInfo &) (xtc->src);  
  
  if (level == Level::Segment)
  {
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );
    const ProcInfo & info = (const ProcInfo &) (xtc->src);
    printf("ip 0x%x pid 0x%x\n", info.ipAddr(), info.processId());
    
    if ( _depth != 0 )
      printf( "XtcIterWithOffset::process(): *** Error depth: Expect 0, but get %d\n", _depth );
  }  
  else if (level == Level::Source)
  {
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );
    printf("src %s,%d %s,%d\n",
     DetInfo::name(info.detector()), info.detId(),
     DetInfo::name(info.device()), info.devId());
     
    if ( _depth != 1 && _depth != 2 )
      printf( "XtcIterWithOffset::process(): *** Error depth: Expect 1 or 2, but get %d\n", _depth );
  }  
  else if (level == Level::Reporter)
  {
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );     
    const BldInfo & info = *(BldInfo*) (&xtc->src);
    printf("pid 0x%x type %s\n", info.processId(), BldInfo::name(info));
    
    if ( _depth != 1 )
      printf( "XtcIterWithOffset::process(): *** Error depth: Expect 1, but get %d\n", _depth );
  }  
  else
  {    
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d\n",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );
  }  

  _i64Offset += xtc->extent;
  ++_iNumXtc;  
  
  if (xtc->contains.id() == TypeId::Id_Xtc)
  {
    XtcIterWithOffset iter(xtc, _depth + 1, i64OffsetPayload);
    iter.iterate();    
    
    if ( iter._iNumXtc > 5 )
    {
      unsigned i = _depth+1;
      while (i--) printf("  ");
      printf( "Xtc Number %d\n", iter._iNumXtc );    
    }
  }
    
  return XtcIterWithOffset::Continue;     
}

int XtcIterConfig::process(Xtc * xtc)
{
  Level::Type     level             = xtc->src.level();
  int64_t         i64OffsetPayload  = _i64Offset + sizeof(Xtc);
  const DetInfo&  info              = (const DetInfo &) (xtc->src);  
  
  if (level == Level::Segment)
  {
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );
    const ProcInfo & info = (const ProcInfo&) (xtc->src);
    printf("ip 0x%x pid 0x%x\n", info.ipAddr(), info.processId());

    if ( _depth != 0 )
      printf( "XtcIterConfig::process(): *** Error depth: Expect 0, but get %d\n", _depth );    
  }  
  else if (level == Level::Source)
  {
    if ( xtc->contains.id() != TypeId::Id_Epics || _iNumXtc < 2 )
    {    
      unsigned i = _depth;
      while (i--) printf("  ");
      printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
       Level::name(level), (long long) _i64Offset, 
       xtc->damage.value(), xtc->extent,
       TypeId::name(xtc->contains.id()), xtc->contains.version()
       );
      printf("src %s,%d %s,%d\n",
       DetInfo::name(info.detector()), info.detId(),
       DetInfo::name(info.device()), info.devId());   
    }
    
    if ( _depth != 1 && _depth != 2 )
      printf( "XtcIterConfig::process(): *** Error depth: Expect 1 or 2, but get %d\n", _depth );
  }
  else if (level == Level::Control)
  {
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d\n",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );
     
    if ( _depth != 1 )
      printf( "XtcIterConfig::process(): *** Error depth: Expect 1, but get %d\n", _depth );
  }
  else if (level == Level::Reporter)
  {
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );     
    const BldInfo & info = *(BldInfo*) (&xtc->src);
    printf("pid 0x%x type %s\n", info.processId(), BldInfo::name(info));
    
    if ( _depth != 2 )
      printf( "XtcIterL1Accept::process(): *** Error depth: Expect 2, but get %d\n", _depth );
  }    
  else
  {
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d\n",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );
     
    printf( "XtcIterConfig::process(): *** Error level %s depth = %d\n", Level::name(level), _depth );     
  }

  _i64Offset += xtc->extent;
        
  ++_iNumXtc;  
  if (xtc->contains.id() == TypeId::Id_Xtc)
  {
    XtcIterConfig iter(xtc, _depth + 1, i64OffsetPayload);
    iter.iterate();
    
    if ( iter._iNumXtc > 5 )
    {
      unsigned i = _depth+1;
      while (i--) printf("  ");
      printf( "Xtc Number %d\n", iter._iNumXtc );    
    }
  }
    
  return XtcIterConfig::Continue;     
}

int XtcIterL1Accept::process(Xtc * xtc)
{
  Level::Type     level             = xtc->src.level();
  int64_t         i64OffsetPayload  = _i64Offset + sizeof(Xtc);
  const DetInfo&  info              = (const DetInfo &) (xtc->src);
  
  if (level == Level::Segment)
  {
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );
    const ProcInfo & info = (const ProcInfo &) (xtc->src);
    printf("ip 0x%x pid 0x%x\n", info.ipAddr(), info.processId());
    
    if ( _depth != 0 )
      printf( "XtcIterL1Accept::process(): *** Error depth: Expect 0, but get %d\n", _depth );
  }  
  else if (level == Level::Source)
  {
    if ( xtc->contains.id() != TypeId::Id_Epics || _iNumXtc < 1 )
    {    
      unsigned i = _depth;
      while (i--) printf("  ");
      printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
       Level::name(level), (long long) _i64Offset, 
       xtc->damage.value(), xtc->extent,
       TypeId::name(xtc->contains.id()), xtc->contains.version()
       );
      printf("src %s,%d %s,%d\n",
       DetInfo::name(info.detector()), info.detId(),
       DetInfo::name(info.device()), info.devId());
    }
  
    if ( _depth != 1 && _depth != 2 )
      printf( "XtcIterL1Accept::process(): *** Error depth: Expect 1 or 2, but get %d\n", _depth );
            
    if ( xtc->contains.id() == TypeId::Id_EvrData )
    {
      unsigned i = _depth;
      while (i--) printf("  ");
      printf("  Evr Events ");
      updateEvr(*xtc);
      printf("\n");
    }
  }  
  else if (level == Level::Reporter)
  {
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );     
    const BldInfo & info = *(BldInfo*) (&xtc->src);
    printf("pid 0x%x type %s\n", info.processId(), BldInfo::name(info));
    
    if ( _depth != 1 )
      printf( "XtcIterL1Accept::process(): *** Error depth: Expect 1, but get %d\n", _depth );
  }  
  else
  {    
    unsigned i = _depth;
    while (i--) printf("  ");
    printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d\n",
     Level::name(level), (long long) _i64Offset, 
     xtc->damage.value(), xtc->extent,
     TypeId::name(xtc->contains.id()), xtc->contains.version()
     );
     
    printf( "XtcIterL1Accept::process(): *** Error level %s depth = %d\n", Level::name(level), _depth );
  }  

  _i64Offset += xtc->extent;
      
  ++_iNumXtc;
  if (xtc->contains.id() == TypeId::Id_Xtc)
  {
    XtcIterL1Accept iter(xtc, _depth + 1, i64OffsetPayload);
    iter.iterate();        
    
    if ( iter._iNumXtc > 5 )
    {
      unsigned i = _depth+1;
      while (i--) printf("  ");
      printf( "Xtc Number %d\n", iter._iNumXtc );    
    }    
  }
    
  return XtcIterL1Accept::Continue;     
}

//int gotoEvent2(char* sIndexFilename, int iEvent, int fdXtc)
//{
//  Index::IndexList indexList;
//  readIndex( sIndexFilename, indexList );
//  
//  Index::L1AcceptNode* pNode = NULL;
//  indexList.getNode( iEvent, pNode );
//    
//  int64_t i64OffsetSeek = lseek64(fdXtc, pNode->i64OffsetXtc, SEEK_SET);
//  if ( i64OffsetSeek != pNode->i64OffsetXtc )
//  {
//    printf( "gotoEvent(): Failed to jump to %d event at offset 0x%Lx (actual offset 0x%Lx)\n",
//      iEvent, pNode->i64OffsetXtc, i64OffsetSeek );
//    lseek64(fdXtc, 0, SEEK_SET);
//    
//    return 1;
//  }
//  
//  return 0;
//}
//
//int readIndex(char* sInputIndex, Index::IndexList& indexList)
//{
//  int fd = open(sInputIndex, O_RDONLY | O_LARGEFILE);
//  if (fd < 0)
//  {
//    printf("Unable to open xtc file %s\n", sInputIndex);
//    return 1;
//  }
//  
//  indexList.readFromFile(fd);
//  
//  ::close(fd);
//  
//  int iVerbose = 1;
//  indexList.printList(iVerbose);  
//      
//  return 0;
//}
