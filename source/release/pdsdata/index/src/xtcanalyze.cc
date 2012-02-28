#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <string>

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/BldInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/index/IndexFileReader.hh"
#include "pdsdata/index/IndexChunkReader.hh"
#include "pdsdata/ana/XtcRun.hh"

using std::vector;
using std::string;
using namespace Pds;
using namespace Ana;

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
    "Usage:  %s  [-f <xtc filename>] [-i <index>] "
    "[-n <output L1 event#>] [-o <offset>] "
    "[-j <begin L1 event#>] [-y <begin calib cycle#>] [-t <time>]\n"
    "[-u <fiducial>[,<event>]] [-m <from_event>] [-h]\n"
    "  Options:\n"
    "    -h                       Show usage.\n"
    "    -f <xtc filename>        Set xtc filename\n"
    "    -i <index filename>      Set index filename\n"
    "    -o <offset>              Start analysis from offset\n"
    "    -n <output L1 event#>    Set L1 event# for ouput\n"
    "    -j <begin L1 event#>     Set begin L1 event#\n"
    "    -y <begin calib cycle#>  Set begin calib cycle#\n"
    "    -t <time>                Go to the event at <time>\n"
    "    -u <fiducial>[,<event>]  Go to the event with fiducial <fiducial>, searching from <event>\n"
    ,    
    progname
  );
}

//forward function declarations
int openXtcRun(const char* sXtcFilename, XtcRun& run);
int genListFromBasename(const char* sXtcFilename, vector<string>& lRunFilename);

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
  uint32_t      uFiducialSearch = (uint32_t) -1;
  int           iFidFromEvent   = 1;

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
    case 'n':
      iNumL1Event   = strtol(optarg, NULL, 0);
      break;
    case 'o':
      i64OffsetStart= strtoll(optarg, NULL, 0);
      break;
    case 'j':
      iBeginL1Event = strtol(optarg, NULL, 0);
      break;
    case 'y':
      iBeginCalib   = strtol(optarg, NULL, 0);
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
    
  return xtcAnalyze( sXtcFilename, sIndexFilename, iBeginL1Event, iNumL1Event, iBeginCalib, 
    i64OffsetStart, sTime, uFiducialSearch, iFidFromEvent );
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
        
    printf( "Calib %d Off 0x%Lx L1 %d %s.%03u\n", iCalib, calibNode.i64Offset, calibNode.iL1Index,
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
  XtcRun run;  
  int iError = openXtcRun(sXtcFilename, run);
  if (iError != 0)
    return 1;

  run.init();
  
  //TEventNoList lEventNo;
  //convertEventRangeToNo(run, lEventRange, lEventNo);
  //unsigned  uNextEventNo    = 0;
  //bool      bLoadNextEvent  = ( lEventNo.size() != 0 );

  if (sTime != NULL)
  {
    int   iCalib = -1, iEvent = -1;
    bool  bExactMatch = false;
    bool  bOvertime   = false;
    int   iError      = run.findTime(sTime, iCalib, iEvent, bExactMatch, bOvertime);
    
    if (iError != 0)
    {
      if (bOvertime)
        printf("Time %s is later than the last event\n", sTime);
      else
        printf("Cannot find event with the specified time %s\n", sTime);
    }
    else
    {      
      iBeginCalib   = iCalib;
      iBeginL1Event = iEvent;
      printf("Going to event with time %s%s : Calib# %d Event# %d\n",
        sTime, (bExactMatch? " [exact]":""),
        iBeginCalib, iBeginL1Event );
    }
  }
  else if (uFiducialSearch != (uint32_t) -1)
  {
    int   iCalib = -1, iEvent = -1;
    int   iError      = run.findNextFiducial(uFiducialSearch, iFidFromEvent, iCalib, iEvent);
    
    if (iError != 0)
      printf("Cannot find event with the fiducial 0x%x , after global event# %d\n", uFiducialSearch, iFidFromEvent);
    else
    {      
      iBeginCalib   = iCalib;
      iBeginL1Event = iEvent;
      printf("Going to event with fiducial 0x%x after global event# %d : Calib# %d Event# %d\n",
        uFiducialSearch, iFidFromEvent,
        iBeginCalib, iBeginL1Event );
    }    
  }
  
  //if ( iBeginCalib < 1 && iBeginL1Event < 1 && bLoadNextEvent )
  //{
  //  iBeginCalib   = lEventNo[uNextEventNo].calib;
  //  iBeginL1Event = lEventNo[uNextEventNo].event;
  //  
  //  ++uNextEventNo;
  //}
  
  Result        r               = OK;
  unsigned      uCurEvent       = 1;
  unsigned int  uCurCalib       = 0;
  unsigned int  uEventCalibBase = 0;
  unsigned      uNumDamage      = 0;
  bool          bJumpedToEvent  = false; 
  unsigned int  uNumProcessed   = 0;
  bool          bEndLoop        = false;
  uint32_t      damagemask      = 0;
  
  do 
  {  
    Dgram*  dg        = NULL;
    int     iSlice    = -1;
    int64_t i64Offset = -1;
    r = run.next(dg, &iSlice, &i64Offset);
    if (r == Error)
      break;      

    uint32_t damage = dg->xtc.damage.value();
    if (damage)
    {
        uNumDamage++;
        damagemask |= damage;
    }

    if (dg->seq.service() == TransitionId::L1Accept)
    {      
      char sTimeBuff[128];
      time_t t = dg->seq.clock().seconds();
      strftime(sTimeBuff,128,"%H:%M:%S",localtime(&t));   
      
      printf( "\n<%d> %s #%d ctl 0x%x vec %d fid 0x%x %s.%03u "
       "offset 0x%Lx env 0x%x damage 0x%x extent 0x%x calib %d event %d\n",
       iSlice, TransitionId::name(dg->seq.service()), uCurEvent, dg->seq.stamp().control(),
       dg->seq.stamp().vector(), dg->seq.stamp().fiducials(), 
       sTimeBuff, (int) (dg->seq.clock().nanoseconds() / 1e6),
       i64Offset, dg->env.value(), dg->xtc.damage.value(), dg->xtc.extent,
       uCurCalib, uCurEvent - uEventCalibBase + 1);    
      
      XtcIterL1Accept iterL1Accept(&(dg->xtc), 0, i64Offset + sizeof(*dg) );
      iterL1Accept.iterate();          
      
      ++uCurEvent;      
      ++uNumProcessed;
      
      if ( iNumL1Event > 0 && (int) uNumProcessed >= iNumL1Event )
        bEndLoop = true;
      
  
      //if ( bLoadNextEvent )
      //{
      //  if (uNextEventNo >= lEventNo.size())
      //    break;
      //  
      //  //printf( "Loading next event %u from list. prev c %u j %u ",
      //  //  uNextEventNo, iBeginCalib, iBeginL1Event ); // !! debug
      //    
      //  iBeginCalib   = lEventNo[uNextEventNo].calib;
      //  iBeginL1Event = lEventNo[uNextEventNo].event;

      //  //printf( "next c %u j %u calibCur %u\n", iBeginCalib, iBeginL1Event, uCurCalib ); //!! debug
      //  
      //  if ( iBeginCalib != uCurCalib )
      //  {
      //    int iEventNumAfterJump;
      //    int iError = run.jump(iBeginCalib, 0, iEventNumAfterJump);
      //    if ( iError == 0 )
      //    {
      //      uCurEvent       = iEventNumAfterJump;
      //      uCurCalib       = iBeginCalib - 1;
      //      uEventCalibBase = uCurEvent;
      //      bJumpedToEvent  = false;
      //    }              
      //  }
      //  else
      //  {
      //    int iEventNumAfterJump;
      //    int iError = run.jump(iBeginCalib, iBeginL1Event, iEventNumAfterJump);
      //    if ( iError == 0 )
      //      uCurEvent = iEventNumAfterJump;
      //  }
      //  
      //  ++uNextEventNo;
      //}         
    }
    else if (dg->seq.service() == TransitionId::Configure)
    {      
      char sDateTimeBuff[128];
      time_t t = dg->seq.clock().seconds();
      strftime(sDateTimeBuff,128,"%Z %a %F %T",localtime(&t));
      
      printf( "\n<%d> %s ctl 0x%x vec %d fid 0x%x %s.%03u "
       "offset 0x%Lx env 0x%x damage 0x%x extent 0x%x\n",
       iSlice, TransitionId::name(dg->seq.service()), dg->seq.stamp().control(),       
       dg->seq.stamp().vector(), dg->seq.stamp().fiducials(), 
       sDateTimeBuff, (int) (dg->seq.clock().nanoseconds() / 1e6),
       i64Offset, dg->env.value(), dg->xtc.damage.value(), dg->xtc.extent);    
      
      // Go through the config data and create the cfgSegList object
      XtcIterConfig iterConfig(&(dg->xtc), 0, i64Offset + sizeof(*dg) );
      iterConfig.iterate();      
    }
    else // if (dg->seq.service() == TransitionId::Configure)
    {
      char sDateTimeBuff[128];
      time_t t = dg->seq.clock().seconds();
      strftime(sDateTimeBuff,128,"%Z %a %F %T",localtime(&t));
      
      printf( "\n<%d> %s ctl 0x%x vec %d fid 0x%x %s.%03u "
       "offset 0x%Lx env 0x%x damage 0x%x extent 0x%x\n",
       iSlice, TransitionId::name(dg->seq.service()), dg->seq.stamp().control(),
       dg->seq.stamp().vector(), dg->seq.stamp().fiducials(), 
       sDateTimeBuff, (int) (dg->seq.clock().nanoseconds() / 1e6),
       i64Offset, dg->env.value(), dg->xtc.damage.value(), dg->xtc.extent);           
       
      XtcIterWithOffset iterDefault(&(dg->xtc), 0, i64Offset + sizeof(*dg) );
      iterDefault.iterate();                   
      
      if (dg->seq.service() == TransitionId::BeginRun)
      {
        int iRunnumber = dg->env.value();
        if (iRunnumber==0) 
          iRunnumber = run.run_number();
          
        bJumpedToEvent = false;
        
        if ( iBeginCalib > 1 )
        {
          int iEventNumAfterJump;
          int iError = run.jump(iBeginCalib, 0, iEventNumAfterJump);
          if ( iError == 0 )
          {
            uCurEvent       = iEventNumAfterJump;
            uCurCalib       = iBeginCalib - 1;
            uEventCalibBase = uCurEvent;
          }
        }
        else if ( iBeginCalib == 0 )
          iBeginCalib = 1;
      }    
      else if (dg->seq.service() == TransitionId::BeginCalibCycle)
      {
        ++uCurCalib;
        uEventCalibBase = uCurEvent;
      }
      else if (dg->seq.service() == TransitionId::Enable)
      {
        if ( iBeginL1Event > 1 && !bJumpedToEvent )
        {            
          printf("xtcAnalyze(): Jumping to calib %d event %d\n", 
            iBeginCalib, iBeginL1Event);
          int iEventNumAfterJump;
          int iError = run.jump(iBeginCalib, iBeginL1Event, iEventNumAfterJump);
          if ( iError == 0 )
            uCurEvent = iEventNumAfterJump;
            
          bJumpedToEvent = true;
        }       
      } // if (dg->seq.service() == TransitionId::Enable)
    } // else // if (dg->seq.service() == TransitionId::Configure)
    
  } while(r == OK && !bEndLoop);
  
  printf("Processed %d events, %d damaged, with damage mask 0x%x.\n", 
    uNumProcessed, uNumDamage, damagemask);
    
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
     Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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
    if ( xtc->contains.id() != TypeId::Id_Epics || _iNumXtc < 1 )
    {    
      unsigned i = _depth;
      while (i--) printf("  ");
      printf("%s level  offset 0x%Lx damage 0x%x extent 0x%x contains %s V%d ",
       Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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
       Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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
     Level::name(level), _i64Offset, 
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

int openXtcRun(const char* sXtcFilename, XtcRun& run)
{
  vector<string> lRunFilename;
  int iError = genListFromBasename(sXtcFilename, lRunFilename);
  if (iError != 0)
    return 1;
    
  if (lRunFilename.size() == 0)
  {
    printf("openXtcRun(): No xtc file found. Input filename = %s\n", sXtcFilename);
    return 2;
  }
    
  ////!!debug
  //for (int iFile = 0; iFile < (int) lRunFilename.size(); ++iFile)
  //{
  //  printf("openXtcRun(): File[%d]: %s\n", iFile, lRunFilename[iFile].c_str());
  //}
  if (lRunFilename.size() > 1)
    printf("Found %d files in this run. First: %s Last: %s\n", 
      lRunFilename.size(), lRunFilename.front().c_str(), lRunFilename.back().c_str());
  else
    printf("Only 1 file in this run: %s\n", 
      lRunFilename.front().c_str());
   
  run.reset(lRunFilename[0]);
  for (int iFile = 1; iFile < (int) lRunFilename.size(); ++iFile)
  {
    if (!run.add_file(lRunFilename[iFile])) 
    {
      printf("openXtcRun(): File[%d] %s doesn't match base filename %s\n",
        iFile, lRunFilename[iFile].c_str(), sXtcFilename);
      return 3;
    }
  }  
  
  return 0;
}

int genListFromBasename(const char* sXtcFilename, vector<string>& lRunFilename)
{
  lRunFilename.clear();
  
  /*
   * Get base filename
   */
  string strFnXtc(sXtcFilename);
  
  size_t uPos = strFnXtc.find("-s");
  if (uPos == string::npos)
  {
    struct ::stat64 statFile;
    int iError = ::stat64(sXtcFilename, &statFile);
    if ( iError != 0 )
    {
      printf("genListFromBasename(): Input filename %s doesn't exists\n", sXtcFilename);
      return 0;
    }
    
    lRunFilename.push_back(strFnXtc);    
    return 0;
  }
  
  string strFnExt = strFnXtc.substr(uPos+9); // Move from "-s00-c00.xtc..." to "xtc..."
    
  string strFnBase = strFnXtc.substr(0, uPos+2);
    
  /*
   * Read index files
   */
  const int MAX_SLICES = 6;
  int iChunk = 0;
  while (true)
  {
    bool bSliceFound = false;
    for (int iSlice = 0; iSlice < MAX_SLICES; ++iSlice)
    {
      char sFnBuf[64];
      sprintf(sFnBuf, "%s%02d-c%02d.%s", strFnBase.c_str(), iSlice, iChunk, strFnExt.c_str());
      
      struct ::stat64 statFile;
      int iError = ::stat64(sFnBuf, &statFile);
      if ( iError != 0 )
        continue;

      lRunFilename.push_back(sFnBuf);
      bSliceFound = true;
    }
    
    if (!bSliceFound)
      break;
      
    ++iChunk;
  }
  
  if (iChunk == 0)
  {
    struct ::stat64 statFile;
    int iError = ::stat64(sXtcFilename, &statFile);
    if ( iError != 0 )
    {
      printf("genListFromBasename(): Input filename %s doesn't exists\n", sXtcFilename);
      return 0;
    }
    
    lRunFilename.push_back(strFnXtc);    
    return 0;
  }   
  
  return 0;
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
