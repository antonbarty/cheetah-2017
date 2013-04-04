/* $Id: main.cc,v 1.111 2012/06/07 16:08:33 weaver Exp $ */
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

//#include <TROOT.h>
//#include <TApplication.h>
//#include <TFile.h>
//#include <TH1.h>

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <map>
#include <algorithm>
#include <list>
#include <iterator>
#include <memory>
#include <poll.h>
#include <signal.h>
#include <sys/stat.h>

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/Src.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/TimeStamp.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/opal1k/ConfigV1.hh"
#include "pdsdata/pulnix/TM6740ConfigV1.hh"
#include "pdsdata/pulnix/TM6740ConfigV2.hh"
#include "pdsdata/fccd/FccdConfigV2.hh"
#include "pdsdata/phasics/ConfigV1.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/camera/TwoDGaussianV1.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/epics/EpicsPvData.hh"
#include "pdsdata/epics/ConfigV1.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "pdsdata/control/ConfigV1.hh"
#include "pdsdata/control/PVControl.hh"
#include "pdsdata/control/PVMonitor.hh"
#include "pdsdata/evr/ConfigV1.hh"
#include "pdsdata/evr/ConfigV2.hh"
#include "pdsdata/evr/ConfigV3.hh"
#include "pdsdata/evr/ConfigV4.hh"
#include "pdsdata/evr/ConfigV5.hh"
#include "pdsdata/evr/ConfigV6.hh"
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/princeton/ConfigV1.hh"
#include "pdsdata/princeton/ConfigV2.hh"
#include "pdsdata/princeton/ConfigV3.hh"
#include "pdsdata/princeton/FrameV1.hh"
#include "pdsdata/princeton/InfoV1.hh"
#include "pdsdata/ipimb/ConfigV1.hh"
#include "pdsdata/ipimb/DataV1.hh"
#include "pdsdata/encoder/ConfigV1.hh"
#include "pdsdata/encoder/ConfigV2.hh"
#include "pdsdata/encoder/DataV1.hh"
#include "pdsdata/encoder/DataV2.hh"
#include "pdsdata/lusi/DiodeFexConfigV1.hh"
#include "pdsdata/lusi/DiodeFexV1.hh"
#include "pdsdata/lusi/IpmFexConfigV1.hh"
#include "pdsdata/lusi/IpmFexV1.hh"
#include "pdsdata/cspad/ConfigV1.hh"
#include "pdsdata/cspad/ConfigV2.hh"
#include "pdsdata/cspad/ConfigV3.hh"
#include "pdsdata/cspad/ConfigV4.hh"
#include "pdsdata/cspad/ElementIterator.hh"
#include "pdsdata/cspad/ElementHeader.hh"
#include "pdsdata/cspad/MiniElementV1.hh"
#include "pdsdata/cspad2x2/ConfigV1.hh"
#include "pdsdata/cspad2x2/ElementV1.hh"
#include "pdsdata/gsc16ai/ConfigV1.hh"
#include "pdsdata/gsc16ai/DataV1.hh"
#include "pdsdata/timepix/ConfigV1.hh"
#include "pdsdata/timepix/ConfigV2.hh"
#include "pdsdata/timepix/DataV1.hh"
#include "pdsdata/timepix/DataV2.hh"
#include "pdsdata/oceanoptics/ConfigV1.hh"
#include "pdsdata/oceanoptics/DataV1.hh"
#include "pdsdata/fli/ConfigV1.hh"
#include "pdsdata/fli/FrameV1.hh"
#include "pdsdata/ana/XtcRun.hh"

#include <main.hh>
#include <myana.hh>
#include <SplitEventQ.cc>

using std::vector;
using std::string;
using std::map;
using std::queue;
using std::auto_ptr;
using namespace Pds;
using namespace Ana;

static string GenerateCtrlPvHashKey( const char* pvName, int arrayIndex);

class XtcWriter {
public:
  XtcWriter(char* path);
  ~XtcWriter();
public:
  void insert(Dgram& dg);
private:
  void _openFile(Dgram&);
  void _write(Dgram&);
  void _queue(Dgram&);
  void _flush(Dgram&);
private:
  std::string       _path;
  FILE*             _ofile;
  std::list<char*>  _tr;
  bool              _event_recorded;
};

//
//  A class to collect pointers to each detector's data from the xtc iteration
//
class EventStore : public XtcIterator {
  enum {Stop, Continue};
  //
  //  The Src key for identifying the detector (ignores processId)
  //
  class Key {
  public:
    Key(const Src& info,TypeId t) :
      _level(info.level()), _info(info.phy()), _t(t) {}
  public:
    bool operator<(const Key& o) const
    {
      if (_level != o._level) return _level < o._level;
      if (_info  != o._info ) return _info  < o._info ;
      return _t.value() < o._t.value();
    }
    unsigned phy () const { return _info; }
    TypeId   type() const { return _t; }
  private:
    unsigned _level;
    unsigned _info;
    TypeId   _t;
  };

public:
  EventStore(const char* reorder_file=0, int iDebugLevel=false) : _iDebugLevel(iDebugLevel), _shift_def(0), _complete(false)
  {
    if (reorder_file) {
      FILE* f = fopen(reorder_file,"r");
      if (f) {
        char line[1024];
        char* lptr = line;
        size_t line_sz=1024;
        while( getline(&lptr,&line_sz,f)!=-1 ) {
          if (lptr[0]=='#') continue;
          unsigned key   = strtoul(lptr,&lptr,16);
          int      shift = strtol (lptr,&lptr,0);
          printf("shift_map[%08x] = %d\n",key,shift);
          _shift_map[key] = shift;
          if (shift < _shift_def)
            _shift_def = shift;
        }
        int shift = -_shift_def;
        while(shift--) {
          _seq_queue.push(Sequence(ClockTime(0,0),TimeStamp(0,0,0)));
        }
      }
      else
        printf("Failed to open %s\n",reorder_file);
    }

    _reset_control();
  }
  ~EventStore() 
  {
    _reset_epics();
    for(map<Key,const Xtc*>::iterator it=_cmaps.begin(); it!=_cmaps.end(); it++)
      delete[] reinterpret_cast<const char*>(it->second);
    for(map<Key,const Xtc*>::iterator it=_bmaps.begin(); it!=_bmaps.end(); it++)
      delete[] reinterpret_cast<const char*>(it->second);
  }
public:
  void processDg(Dgram* dg) 
  {
    _seq = dg->seq;

    if (_seq.service()==TransitionId::Configure) {
      _smaps.clear();
      _reset_epics ();
    }

    if (_seq.service()==TransitionId::BeginRun)
      _clear_epics ();

    if (_seq.service()==TransitionId::BeginCalibCycle)
      _reset_control();

    if (_seq.service()==TransitionId::L1Accept) {
      _rmaps.clear();
      _clear_epics ();
      _clear_shift();
    }

    iterate(&dg->xtc);

    if (_seq.service()==TransitionId::L1Accept) {
      _shift();
      _complete |= dg->xtc.damage.value()==0;
      _seq_queue.push(_seq);
      _seq = _seq_queue.front();
      _seq_queue.pop();
    }
  }
public:
  const Sequence& sequence() const { return _seq; }
  //
  //  Find the detector data of TypeId from Src
  //
  const Xtc* lookup_cfg(const Src& info, TypeId t) const { return _lookup(info,t,_cmaps); }
  const Xtc* lookup_evt(const Src& info, TypeId t) const { return _lookup(info,t,_rmaps); }
  //
  //  Find the epics data
  //
  const vector<const EpicsPvHeader*>& epics() const { return _repics; }
  int   epics_index(const char* pvName) 
  {
    return ( _epics_name_map.find( pvName )==_epics_name_map.end() ) ?
      -1 : _epics_name_map[ pvName ];
  }
  int   getPvConfig(int iPvId, const char*& pvDescription, double& updateInterval)
  {
    if ( iPvId < 0 || iPvId >= (int) _epics_config.size() )
      return 1;
      
    pvDescription   = _epics_config[iPvId].sPvDesc;
    updateInterval  = _epics_config[iPvId].fInterval;
    return 0;
  }
  
  //
  //  Find the control data
  //
  const ControlData::ConfigV1& control_data() const 
  { return *reinterpret_cast<const ControlData::ConfigV1*>(&_control_buffer[0]); }
  int   control_index(const char* pvName, int arrayIndex) const { 
    string pvHashKey = GenerateCtrlPvHashKey(pvName, arrayIndex);
    map<string,int>::const_iterator it=_control_name_map.find( pvHashKey );
    return ( it==_control_name_map.end() ) ? -1 : it->second;
  }
  int   monitor_index(const char* pvName, int arrayIndex) const { 
    string pvHashKey = GenerateCtrlPvHashKey(pvName, arrayIndex);
    map<string,int>::const_iterator it=_monitor_name_map.find( pvHashKey );
    return ( it==_monitor_name_map.end() ) ? -1 : it->second;
  }
  //
  //  Fetch static buffer for detector "transformation"
  //
  const Xtc* buffer(const Src& info, TypeId t, unsigned size) {
    const Xtc* xtc;
    Key key(info,t);
    map<Key,const Xtc*>::const_iterator it = _bmaps.find(key);
    if (it == _bmaps.end()) {
      char* p = new char[sizeof(Xtc)+size];
      Xtc* nxtc = new (p) Xtc(t,info);
      nxtc->alloc(size);
      _bmaps[key] = xtc = reinterpret_cast<const Xtc*>(p);
    }
    else {
      xtc = it->second;
    }
    return xtc;
  }
private:
  int process(Xtc* xtc) 
  {
    if (xtc->extent        <  sizeof(Xtc) ||        // check for corrupt data
        xtc->contains.id() >= TypeId::NumberOf) {
      return Stop;
    }
    else if (xtc->contains.id() == TypeId::Id_Xtc) { // iterate through hierarchy
      iterate(xtc);
    }
    else {                          // handle specific detector data
      if (_iDebugLevel >= 1) {
        const DetInfo& info = *(DetInfo*)(&xtc->src);
        printf( "> Xtc type %-16s  Detector %-12s:%d  Device %-12s:%d\n", 
                TypeId::name(xtc->contains.id()),
                DetInfo::name(info.detector()), info.detId(), 
                DetInfo::name(info.device()), info.devId() );
      }

      switch(xtc->contains.id()) {
      case TypeId::Id_ControlConfig:
        _store_control(xtc);
        break;
      case TypeId::Id_Epics:
        _store_epics(xtc);
        break;
      case TypeId::Id_EpicsConfig:
        _store_epics_config(xtc);
        break;
      case TypeId::Id_AcqWaveform:  // Kludge to handle mis-assignment
      case TypeId::Id_AcqTdcData:   //  of Acqiris damage to container
        if ((xtc-1)->damage.value()) 
          xtc->damage.increase((xtc-1)->damage.value());
      default:
        _store(xtc->src, xtc->contains, xtc);
        break;
      }
    }
    return Continue;
  }
private:
  const Xtc* _lookup(const Src& info, TypeId t, const map<Key,const Xtc*>& maps) const
  {
    Key key(info,t);
    map<Key,const Xtc*>::const_iterator it = maps.find(key);
    return (it == maps.end()) ? 0 : it->second;
  }
private:
  //
  //  Store the data of TypeId from Src
  //  For non-L1A transitions, store a copy
  //
  void _store(Src info,TypeId t,const Xtc* xtc) 
  {
    Key key(info,t);
    if (_seq.service()==TransitionId::L1Accept) {
      _rmaps[key] = xtc;
    }
    else {
      const Xtc* oxtc = _lookup(info,t,_cmaps);
      if (oxtc) {
        delete[] reinterpret_cast<const char*>(oxtc);
        _cmaps.erase(Key(info,t));
      }
      char* nxtc = new char[xtc->extent];
      memcpy(nxtc,xtc,xtc->extent);
      _cmaps[key] = reinterpret_cast<const Xtc*>(nxtc);
    }
  }
  void _store_epics(const Xtc* xtc) 
  {
    //
    //  Lookup id map for this xtc source,
    //  If missing, create
    //
    map<int,vector<int>*>::const_iterator it=_epics_xtc_map.find( xtc->src.phy() );
    if ( it==_epics_xtc_map.end()) {
      printf("Creating map for epics src %08x.%08x\n",
             xtc->src.log(),xtc->src.phy());
      _epics_xtc_map[ xtc->src.phy() ] = new vector<int>;
    }
    vector<int>& id_map = *_epics_xtc_map[ xtc->src.phy() ];

    const EpicsPvHeader& epicsPv = *reinterpret_cast<const EpicsPvHeader*>(xtc->payload());

    if ( epicsPv.iPvId >= (int) id_map.size() ) {
      id_map.resize( epicsPv.iPvId+1 );
      id_map[ epicsPv.iPvId ] = _repics.size();
      _repics.resize( _repics.size()+1 ); 
    }

    _repics[ id_map[epicsPv.iPvId] ] = &epicsPv;

    if ( dbr_type_is_CTRL(epicsPv.iDbrType) ) {
      const char* pvName = ((const EpicsPvCtrlHeader&)epicsPv).sPvName;
      if ( epicsPv.iPvId < (int)id_map.size()-1 && _epics_config.size() == 0)
        printf( "EventStore::_store_epics(): epics control data (%s id %d) is duplicated\n", pvName, epicsPv.iPvId );
      _epics_name_map[ pvName ] = id_map[ epicsPv.iPvId ];
    }
  }
  void _store_epics_config(const Xtc* xtc) 
  {
    //
    //  Lookup id map for this xtc source,
    //  If missing, create
    //
    map<int,vector<int>*>::const_iterator it=_epics_xtc_map.find( xtc->src.phy() );
    if ( it==_epics_xtc_map.end()) {
      printf("(config) Creating map for epics src %08x.%08x\n",
             xtc->src.log(),xtc->src.phy());
      _epics_xtc_map[ xtc->src.phy() ] = new vector<int>;
    }
    vector<int>& id_map = *_epics_xtc_map[ xtc->src.phy() ];

    const Epics::ConfigV1& epicsConfig = *reinterpret_cast<const Epics::ConfigV1*>(xtc->payload());
    int iNumPv = epicsConfig.getNumPv();    
    for (int iPv = 0; iPv < iNumPv; ++iPv)
    {
      Epics::PvConfigV1& config = *epicsConfig.getPvConfig(iPv);
      
      if ( config.iPvId < (int) id_map.size() )
      {
        printf("EventStore::_store_epics_config(): PV index %d duplicated\n", config.iPvId);
        continue;
      }
      
      id_map.resize( config.iPvId+1 );
      
      id_map[config.iPvId] = _repics.size();      
      _epics_config.resize(_repics.size()+1);
      _epics_config[_repics.size()] = config;   
      _repics.resize( _repics.size()+1 ); 
      _epics_name_map[ string(config.sPvDesc) ] = id_map[ config.iPvId ];      
    }
  }
  void _reset_epics()
  {
    for(map<int,vector<int>*>::iterator it=_epics_xtc_map.begin(); it!=_epics_xtc_map.end(); it++) {
      vector<int>* vp = (*it).second;
      delete vp;
    }
    _epics_xtc_map .clear();
    _epics_name_map.clear();
  }
  void _clear_epics()
  {
    for(int i=0; i<(int)_repics.size(); i++)
      _repics[i] = 0;
  }
  void _reset_control()
  {
    _control_buffer.resize(sizeof(ControlData::ConfigV1));
    *new (&_control_buffer[0]) ControlData::ConfigV1(ControlData::ConfigV1::Default);

    _control_name_map.clear();
    _monitor_name_map.clear();
  }
  void _store_control(const Xtc* xtc)
  {
    const ControlData::ConfigV1& config = *reinterpret_cast<const ControlData::ConfigV1*>(xtc->payload());
    _control_buffer.resize( config.size() );
    *new ( &_control_buffer[0] ) ControlData::ConfigV1(config);
    
    for(unsigned int iPvControl=0; iPvControl < config.npvControls(); iPvControl++) 
      {
        const Pds::ControlData::PVControl& pvControlCur = config.pvControl(iPvControl);      
        _control_name_map[ GenerateCtrlPvHashKey( pvControlCur.name(), pvControlCur.index() ) ] = iPvControl;
      }

    for(unsigned int iPvMonitor=0; iPvMonitor < config.npvMonitors(); iPvMonitor++) 
      {      
        const Pds::ControlData::PVMonitor& pvMonitorCur = config.pvMonitor(iPvMonitor);
        _monitor_name_map[ GenerateCtrlPvHashKey( pvMonitorCur.name(), pvMonitorCur.index() ) ] = iPvMonitor;
      }
  }
  void _shift()
  {
    //
    //  Create the queues of shifted data for detectors that haven't yet been seen
    //
    if (!_complete) {
      for(map<Key,const Xtc*>::iterator it=_rmaps.begin(); it!=_rmaps.end(); it++) {
        const Key& key = it->first;
        map<Key,queue<const Xtc*>*>::iterator sit = _smaps.find(it->first);
        if (sit == _smaps.end()) {
          int shift = -_shift_def;
          map<unsigned,int>::const_iterator rit = _shift_map.find(key.phy());
          if ( rit != _shift_map.end() )
            shift += rit->second;

          if (shift) {
            // printf("registering queue depth %d for %08x/%08x\n",shift,key.phy(),key.type().value());
            queue<const Xtc*>* q = new queue<const Xtc*>;
            do {
              q->push(0);
            } while(--shift);
            _smaps[key] = q;
          }
        }
      }
    }

    //
    //  Push the current event's data for each shifted detector onto its queue
    //  and copy the data from the front of the queue into the current event
    //
    for(map<Key,queue<const Xtc*>*>::iterator it=_smaps.begin(); it!=_smaps.end(); it++) {
      map<Key,const Xtc*>::iterator rit = _rmaps.find(it->first);
      if (rit == _rmaps.end())
        it->second->push(0);
      else {
        const Xtc* xtc = rit->second;
        if (xtc) {
          char* b = new char[xtc->extent];
          memcpy(b,xtc,xtc->extent);
          it->second->push(reinterpret_cast<const Xtc*>(b));
          rit->second = it->second->front();
          // printf("pushed %p -> %p for %08x/%08x\n",b,rit->second,it->first.phy(),it->first.type().value());
        }
        else
          it->second->push(0);
      }
    }
  }
  void _clear_shift()
  {
    //
    //  Remove the front of each shifted detector's queue which was copied
    //  into the previous event
    //
    for(map<Key,queue<const Xtc*>*>::iterator it=_smaps.begin(); it!=_smaps.end(); it++) {
      const Xtc* xtc = it->second->front();
      if (xtc) 
        delete reinterpret_cast<char*>(const_cast<Xtc*>(xtc));
      it->second->pop();
      // printf("popped %p for %08x\n",xtc,it->first.phy());
    }
  }

private:
  int      _iDebugLevel;
  Sequence _seq;
  // xtc data
  map<Key,const Xtc*> _rmaps;  // reference to data
  map<Key,const Xtc*> _cmaps;  // copy of data
  // epics data
  map<int,vector<int>*>        _epics_xtc_map;  // mapping pvId for each xtc source to aggregate pvId
  map<string,int>              _epics_name_map; // mapping name to aggregate pvId  
  vector<const EpicsPvHeader*> _repics;         // reference to epics data
  vector<Epics::PvConfigV1>    _epics_config;
  // calib data
  vector<unsigned char> _control_buffer;
  map<string, int>      _control_name_map;
  map<string, int>      _monitor_name_map;
  // processed data
  map<Key,const Xtc*> _bmaps;
  // shifted data
  int                         _shift_def;
  map<unsigned,int>           _shift_map;
  queue<Sequence>             _seq_queue;
  map<Key,queue<const Xtc*>*> _smaps;
  bool _complete;
};

static EventStore* _estore;

void fillConstFrac(double* t, double* v, unsigned numSamples, float baseline,
                   float thresh, double* edge, int& n, int maxhits) {
  // find the boundaries where the pulse crosses the threshold
  n = 0;
  double   peak=0.0;
  unsigned start  =0;
  bool     crossed=false;
  bool     rising = thresh > baseline;
  for(unsigned k=0; k<numSamples; k++) {
    double y = v[k];
    bool over = 
      ( rising && y>thresh) ||
      (!rising && y<thresh);
    if (!crossed && over) {
      crossed = true;
      start   = k;
      peak    = y;
    }
    else if (crossed && !over) {
      //  find the edge
      double edge_v = 0.5*(peak+baseline);
      unsigned i=start;
      if (rising) { // leading edge +
        while(v[i] < edge_v)
          i++;
      }
      else {        // leading edge -
        while(v[i] > edge_v)
          i++;
      }

      if (i>0)
        edge[n] = ((edge_v-v[i-1])*t[i] - (edge_v-v[i])*t[i-1])/(v[i]-v[i-1]);
      else
        edge[n] = t[0];
      if (++n >= maxhits)
        break;
      crossed = false;
    }
    else if (( rising && y>peak) ||
             (!rising && y<peak))
      peak = y;
  }
}

/*void fillConstFrac(double* t, double* v, unsigned numSamples, float baseline,
                   float thresh, TH1* hist) {
  double edge[100];
  int n;
  fillConstFrac(t,v,numSamples,baseline,thresh,edge,n,100);
  for(int i=0; i<n; i++)
    hist->Fill(edge[i],1.0);
}
*/

/* 
 * Time Data
 */

int getTime( int& seconds, int& nanoSeconds )
{
  seconds     = _estore->sequence().clock().seconds();
  nanoSeconds = _estore->sequence().clock().nanoseconds();
  return 0;
}
 
int getLocalTime( const char*& time )
{
  static const char timeFormatStr[40] = "%04Y-%02m-%02d %02H:%02M:%02S"; /* Time format string */    
  static char sTimeText[40];
    
  int seconds = _estore->sequence().clock().seconds();
  struct tm tmTimeStamp;
  localtime_r( (const time_t*) (void*) &seconds, &tmTimeStamp );    
  strftime(sTimeText, sizeof(sTimeText), timeFormatStr, &tmTimeStamp );
    
  time = sTimeText;
  return 0;
}

int getFiducials(unsigned& fiducials)
{
  fiducials = _estore->sequence().stamp().fiducials();
  return 0;
}

static int _runnumber;
int getRunNumber()
{
  return _runnumber;
}

static string _xtcfilename;
string getXTCFilename(){
  return _xtcfilename;
}

/*
  predefined constants for PnCCD camera:
  4 links, each link provides a 512 x 512 x 16 bit image
  Later the getPnCcdValue() function will combine these four images to a single 1024 x 1024 x 16 bit image
*/
static const int      iPnCcdNumLinks         = 4; 
static const int      iPnCcdWidthPerLink     = 512;
static const int      iPnCcdHeightPerLink    = 512;
static const int      iPnCcdImageSizePerLink = iPnCcdWidthPerLink * iPnCcdHeightPerLink * 2; // 2 bytes (16 bit) for each image pixel
static const int      iPnCcdPayloadSize      = iPnCcdImageSizePerLink + sizeof(PNCCD::FrameV1);
static const int      iPnCcdWidth            = 1024;
static const int      iPnCcdHeight           = 1024;
static const int      iPnCcdImageSize        = iPnCcdWidth * iPnCcdHeight * 2; // 2 bytes (16 bit) for each image pixel
//static unsigned char  imagePnCcd[iPnCcdImageSize];

static unsigned jobbegun=0;

static const int FNAME_LEN=128;
static char* currlistname=0;

/*
 * L1Accept data
 */
struct AcqWaveForm
{
  double         vfTrig;
  vector<double> vfTime;
  vector<double> vfVoltage;
};
static vector<AcqWaveForm> AcqChannelList;

static DetInfo AcqDetectorIndex(AcqDetector index)
{
#define ACQ_INFO(t,d) DetInfo(0,DetInfo::t,0,DetInfo::Acqiris,d)
  static const DetInfo info[] = { ACQ_INFO(AmoIms,0),
                                  ACQ_INFO(AmoGasdet,0),
                                  ACQ_INFO(AmoETof,0),
                                  ACQ_INFO(AmoITof,0),
                                  ACQ_INFO(AmoMbes,0),
                                  ACQ_INFO(AmoVmi,0),
                                  ACQ_INFO(AmoBps,0),
                                  ACQ_INFO(Camp,0),
                                  ACQ_INFO(SxrBeamline,0),
                                  ACQ_INFO(SxrBeamline,1),
                                  ACQ_INFO(SxrEndstation,0),
                                  ACQ_INFO(SxrEndstation,1) };
  return info[index];
}

static DetInfo FrameDetectorIndex(FrameDetector det)
{
#define OPAL_INFO(t,d) DetInfo(0,DetInfo::t,0,DetInfo::Opal1000,d)
#define FCCD_INFO(t,d) DetInfo(0,DetInfo::t,0,DetInfo::Fccd,d)
#define PLNX_INFO(t,d) DetInfo(0,DetInfo::t,0,DetInfo::TM6740,d)
#define SID4_INFO(t,d) DetInfo(0,DetInfo::t,0,DetInfo::Phasics,d)
  static const DetInfo info[] = { OPAL_INFO(AmoVmi,0),
                                  OPAL_INFO(AmoBps,0),
                                  OPAL_INFO(AmoBps,1),
                                  OPAL_INFO(SxrBeamline,0),
                                  OPAL_INFO(SxrBeamline,1),
                                  OPAL_INFO(SxrEndstation,0),
                                  OPAL_INFO(SxrEndstation,1),
                                  FCCD_INFO(SxrEndstation,0),
                                  PLNX_INFO(XppSb1Pim,0),
                                  PLNX_INFO(XppMonPim,0),
                                  PLNX_INFO(XppSb3Pim,0),
                                  PLNX_INFO(XppSb4Pim,0),
                                  OPAL_INFO(XppEndstation,0),
                                  SID4_INFO(MecEndstation,0) };
  return info[det];
}

/*
 * Configure data retrieval functions
 */
#define TYPEID(t,v) TypeId(TypeId::Id_##t,v)

int getAcqConfig(DetInfo det, int& numChannels, int& numSamples, double& sampleInterval)
{
  Xtc* xtc = const_cast<Xtc*>(_estore->lookup_cfg( det, TYPEID(AcqConfig,1) ));
  if (xtc && xtc->damage.value()==0) {
    Acqiris::ConfigV1& acqCfg = *reinterpret_cast<Acqiris::ConfigV1*>(xtc->payload());
    const Acqiris::HorizV1& horiz = acqCfg.horiz();
    
    numChannels = acqCfg.nbrChannels();
    numSamples = horiz.nbrSamples();
    sampleInterval = horiz.nbrSamples() * horiz.sampInterval();    
    return 0;
  }
  return xtc ? xtc->damage.value() : 2;
}

int getAcqConfig(AcqDetector det, int& numChannels, int& numSamples, double& sampleInterval)
{
  return getAcqConfig( AcqDetectorIndex(det), numChannels, numSamples, sampleInterval );
}

 
int getFrameConfig(DetInfo info)
{
  const Xtc* xtc = _estore->lookup_cfg( info, TYPEID(FrameFexConfig,1) );
  return xtc ? xtc->damage.value() : 2;
}

int getFrameConfig(FrameDetector det)
{
  return getFrameConfig( FrameDetectorIndex(det) );
}

int getFccdConfig(FrameDetector det, uint16_t& outputMode, bool& ccdEnable, bool& focusMode, uint32_t& exposureTime,
                  float& dacVoltage1, float& dacVoltage2, float& dacVoltage3, float& dacVoltage4,
                  float& dacVoltage5, float& dacVoltage6, float& dacVoltage7, float& dacVoltage8,
                  float& dacVoltage9, float& dacVoltage10, float& dacVoltage11, float& dacVoltage12,
                  float& dacVoltage13, float& dacVoltage14, float& dacVoltage15, float& dacVoltage16,
                  float& dacVoltage17,
                  uint16_t& waveform0, uint16_t& waveform1, uint16_t& waveform2, uint16_t& waveform3,
                  uint16_t& waveform4, uint16_t& waveform5, uint16_t& waveform6, uint16_t& waveform7,
                  uint16_t& waveform8, uint16_t& waveform9, uint16_t& waveform10, uint16_t& waveform11,
                  uint16_t& waveform12, uint16_t& waveform13, uint16_t& waveform14)
{
  const Xtc* xtc = _estore->lookup_cfg( FrameDetectorIndex(det), TYPEID(FccdConfig,2) );
  if (xtc && xtc->damage.value()==0) {
    const FCCD::FccdConfigV2& fccdConfig = *reinterpret_cast<const FCCD::FccdConfigV2*>(xtc->payload());
    outputMode = fccdConfig.outputMode();
    ccdEnable = fccdConfig.ccdEnable();
    focusMode = fccdConfig.focusMode();
    exposureTime = fccdConfig.exposureTime();
    dacVoltage1 = fccdConfig.dacVoltage1();
    dacVoltage2 = fccdConfig.dacVoltage2();
    dacVoltage3 = fccdConfig.dacVoltage3();
    dacVoltage4 = fccdConfig.dacVoltage4();
    dacVoltage5 = fccdConfig.dacVoltage5();
    dacVoltage6 = fccdConfig.dacVoltage6();
    dacVoltage7 = fccdConfig.dacVoltage7();
    dacVoltage8 = fccdConfig.dacVoltage8();
    dacVoltage9 = fccdConfig.dacVoltage9();
    dacVoltage10 = fccdConfig.dacVoltage10();
    dacVoltage11 = fccdConfig.dacVoltage11();
    dacVoltage12 = fccdConfig.dacVoltage12();
    dacVoltage13 = fccdConfig.dacVoltage13();
    dacVoltage14 = fccdConfig.dacVoltage14();
    dacVoltage15 = fccdConfig.dacVoltage15();
    dacVoltage16 = fccdConfig.dacVoltage16();
    dacVoltage17 = fccdConfig.dacVoltage17();
    waveform0 = fccdConfig.waveform0();
    waveform1 = fccdConfig.waveform1();
    waveform2 = fccdConfig.waveform2();
    waveform3 = fccdConfig.waveform3();
    waveform4 = fccdConfig.waveform4();
    waveform5 = fccdConfig.waveform5();
    waveform6 = fccdConfig.waveform6();
    waveform7 = fccdConfig.waveform7();
    waveform8 = fccdConfig.waveform8();
    waveform9 = fccdConfig.waveform9();
    waveform10 = fccdConfig.waveform10();
    waveform11 = fccdConfig.waveform11();
    waveform12 = fccdConfig.waveform12();
    waveform13 = fccdConfig.waveform13();
    waveform14 = fccdConfig.waveform14();
  }
  return xtc ? xtc->damage.value() : 2;
}

int getDiodeFexConfig (DetInfo::Detector det, int iDevId, float* base, float* scale)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Ipimb,0), TypeId(TypeId::Id_DiodeFexConfig,1) );
  if (xtc && xtc->damage.value()==0) {
    const Lusi::DiodeFexConfigV1* p = reinterpret_cast<const Lusi::DiodeFexConfigV1*>(xtc->payload());
    for(unsigned i=0; i<Lusi::DiodeFexConfigV1::NRANGES; i++) {
      base [i] = p->base [i];
      scale[i] = p->scale[i];
    }
  }
  return xtc ? xtc->damage.value() : 2;
}

int getIpmFexConfig   (DetInfo::Detector det, int iDevId, 
                       float* base0, float* scale0,
                       float* base1, float* scale1,
                       float* base2, float* scale2,
                       float* base3, float* scale3,
                       float& xscale, float& yscale)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Ipimb,0), 
                                        TypeId(TypeId::Id_IpmFexConfig,1) );
  if (xtc && xtc->damage.value()==0) {
    const Lusi::IpmFexConfigV1* p = reinterpret_cast<const Lusi::IpmFexConfigV1*>(xtc->payload());
    float* barray[] = { base0 , base1 , base2 , base3  };
    float* sarray[] = { scale0, scale1, scale2, scale3 };
    for(unsigned j=0; j<Lusi::IpmFexConfigV1::NCHANNELS; j++) {
      const Lusi::DiodeFexConfigV1& c = p->diode[j];
      float* base  = barray[j];
      float* scale = sarray[j];
      for(unsigned i=0; i<Lusi::DiodeFexConfigV1::NRANGES; i++) {
        base [i] = c.base [i];
        scale[i] = c.scale[i];
      }
    }
    xscale = p->xscale;
    yscale = p->yscale;
  }
  return xtc ? xtc->damage.value() : 2;
}


int getCspadConfig (DetInfo::Detector det, CsPad::ConfigV1& cfg)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad,0), 
                                        TypeId(TypeId::Id_CspadConfig,1) );
  if (xtc && xtc->damage.value()==0) {
    cfg = *reinterpret_cast<const CsPad::ConfigV1*>(xtc->payload());
  }
  return xtc ? xtc->damage.value() : 2;
}


int getCspadConfig (DetInfo::Detector det, CsPad::ConfigV2& cfg)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad,0), 
                                        TypeId(TypeId::Id_CspadConfig,2) );
  if (xtc && xtc->damage.value()==0) {
    cfg = *reinterpret_cast<const CsPad::ConfigV2*>(xtc->payload());
  }
  return xtc ? xtc->damage.value() : 2;
}


int getCspadConfig (DetInfo::Detector det, CsPad::ConfigV3& cfg)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad,0), 
                                        TypeId(TypeId::Id_CspadConfig,3) );
  if (xtc && xtc->damage.value()==0) {
    cfg = *reinterpret_cast<const CsPad::ConfigV3*>(xtc->payload());
  }
  return xtc ? xtc->damage.value() : 2;
}


int getCspadConfig (DetInfo::Detector det, CsPad::ConfigV4& cfg)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad,0), 
                                        TypeId(TypeId::Id_CspadConfig,4) );
  if (xtc && xtc->damage.value()==0) {
    cfg = *reinterpret_cast<const CsPad::ConfigV4*>(xtc->payload());
  }
  return xtc ? xtc->damage.value() : 2;
}


int getCspad2x2Config (DetInfo::Detector det, CsPad::ConfigV3& cfg)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad2x2,0), 
                                        TypeId(TypeId::Id_CspadConfig,3) );
  if (xtc && xtc->damage.value()==0) {
    cfg = *reinterpret_cast<const CsPad::ConfigV3*>(xtc->payload());
  }
  return xtc ? xtc->damage.value() : 2;
}

int getCspad2x2Config (DetInfo::Detector det, CsPad2x2::ConfigV1& cfg)
{
  return getCspad2x2Config(det, 0, cfg);
}

int getCspad2x2Config (DetInfo::Detector det, int iDevId, CsPad2x2::ConfigV1& cfg)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad2x2,iDevId), 
                                        TypeId(TypeId::Id_Cspad2x2Config,1) );
  if (xtc && xtc->damage.value()==0) {
    cfg = *reinterpret_cast<const CsPad2x2::ConfigV1*>(xtc->payload());
  }
  return xtc ? xtc->damage.value() : 2;
}


int getEpicsPvNumber()
{
  return _estore->epics().size();
}

int getEpicsPvConfig( int pvId, const char*& pvName, int& type, int& numElements )
{
  if ( pvId < 0 || pvId >= (int) _estore->epics().size() ) return 1;
  if ( _estore->epics()[ pvId ] == 0 ) return 2;

  const EpicsPvCtrlHeader& epicsPv = *static_cast<const EpicsPvCtrlHeader*>( _estore->epics()[ pvId ] );
  pvName = epicsPv.sPvName;
  type = epicsPv.iDbrType - DBR_CTRL_DOUBLE + DBR_DOUBLE; // convert from ctrl type to basic type
  numElements = epicsPv.iNumElements;
    
  return 0;
}

int getPrincetonConfig(DetInfo::Detector det, int iDevId, int& width, int& height, int& orgX, int& orgY, int& binX, int&binY)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Princeton,iDevId),
                                        TypeId (TypeId::Id_PrincetonConfig,1) );
  if (xtc) 
  {    
    if ( xtc->damage.value() != 0 )
      return xtc->damage.value();
    
    const Princeton::ConfigV1& princetonCfg = *reinterpret_cast<const Princeton::ConfigV1*>(xtc->payload());
    width   = princetonCfg.width  ();
    height  = princetonCfg.height ();
    orgX    = princetonCfg.orgX   ();
    orgY    = princetonCfg.orgY   ();
    binX    = princetonCfg.binX   ();
    binY    = princetonCfg.binY   ();
    
    return 0;
  }
  
  xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Princeton,iDevId),  
                                        TypeId (TypeId::Id_PrincetonConfig,2) );  
  if (xtc) 
  {    
    if ( xtc->damage.value() != 0 )
      return xtc->damage.value();
    
    const Princeton::ConfigV2& princetonCfg = *reinterpret_cast<const Princeton::ConfigV2*>(xtc->payload());
    width   = princetonCfg.width  ();
    height  = princetonCfg.height ();
    orgX    = princetonCfg.orgX   ();
    orgY    = princetonCfg.orgY   ();
    binX    = princetonCfg.binX   ();
    binY    = princetonCfg.binY   ();
    
    return 0;
  }

  xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Princeton,iDevId),  
                                        TypeId (TypeId::Id_PrincetonConfig,3) );  
  if (xtc) 
  {    
    if ( xtc->damage.value() != 0 )
      return xtc->damage.value();
    
    const Princeton::ConfigV3& princetonCfg = *reinterpret_cast<const Princeton::ConfigV3*>(xtc->payload());
    width   = princetonCfg.width  ();
    height  = princetonCfg.height ();
    orgX    = princetonCfg.orgX   ();
    orgY    = princetonCfg.orgY   ();
    binX    = princetonCfg.binX   ();
    binY    = princetonCfg.binY   ();
    
    return 0;
  }
  
  return 2; // no princeton config found
}

int getOceanOpticsConfig(Pds::DetInfo::Detector det, int iDevId, float& fExposureTime)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::OceanOptics,iDevId),
                                        TypeId (TypeId::Id_OceanOpticsConfig,1) );
                                        
  if (xtc == NULL)
    return 1;
    
  if ( xtc->damage.value() != 0 )
    return xtc->damage.value();
  
  const OceanOptics::ConfigV1& oceanOpticsCfg = *reinterpret_cast<const OceanOptics::ConfigV1*>(xtc->payload());
  fExposureTime   = oceanOpticsCfg.exposureTime();
  
  return 0;
}

int getPrincetonMoreConfig(DetInfo::Detector det, int iDevId, float& exposureTime, float& coolingTemp, int& gain, int& readoutSpeed)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Princeton,iDevId),
                                        TypeId (TypeId::Id_PrincetonConfig,1) );
  if (xtc ) 
  {    
    if ( xtc->damage.value() != 0 )
      return xtc->damage.value();
    
    const Princeton::ConfigV1& princetonCfg = *reinterpret_cast<const Princeton::ConfigV1*>(xtc->payload());
    exposureTime  = princetonCfg.exposureTime     ();
    coolingTemp   = princetonCfg.coolingTemp      ();
    gain          = -1;
    readoutSpeed  = princetonCfg.readoutSpeedIndex();
    
    return 0;
  }
  
  xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Princeton,iDevId),  
                                        TypeId (TypeId::Id_PrincetonConfig,2) );  
  if (xtc ) 
  {    
    if ( xtc->damage.value() != 0 )
      return xtc->damage.value();
    
    const Princeton::ConfigV2& princetonCfg = *reinterpret_cast<const Princeton::ConfigV2*>(xtc->payload());
    exposureTime  = princetonCfg.exposureTime     ();
    coolingTemp   = princetonCfg.coolingTemp      ();
    gain          = princetonCfg.gainIndex        ();
    readoutSpeed  = princetonCfg.readoutSpeedIndex();
    
    return 0;
  }

  xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Princeton,iDevId),  
                                        TypeId (TypeId::Id_PrincetonConfig,3) );  
  if (xtc ) 
  {    
    if ( xtc->damage.value() != 0 )
      return xtc->damage.value();
    
    const Princeton::ConfigV3& princetonCfg = *reinterpret_cast<const Princeton::ConfigV3*>(xtc->payload());
    exposureTime  = princetonCfg.exposureTime     ();
    coolingTemp   = princetonCfg.coolingTemp      ();
    gain          = princetonCfg.gainIndex        ();
    readoutSpeed  = princetonCfg.readoutSpeedIndex();
    
    return 0;
  }
  
  return 2; // no princeton config found
}

int getFliConfig(DetInfo::Detector det, int iDevId, int& width, int& height, int& orgX, int& orgY, int& binX, int&binY)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Fli,iDevId),
                                        TypeId (TypeId::Id_FliConfig,1) );
  if (xtc) 
  {    
    if ( xtc->damage.value() != 0 )
      return xtc->damage.value();
    
    const Fli::ConfigV1& fliCfg = *reinterpret_cast<const Fli::ConfigV1*>(xtc->payload());
    width   = fliCfg.width  ();
    height  = fliCfg.height ();
    orgX    = fliCfg.orgX   ();
    orgY    = fliCfg.orgY   ();
    binX    = fliCfg.binX   ();
    binY    = fliCfg.binY   ();
    
    return 0;
  }
    
  return 2; // no fli config found
}

int getFliMoreConfig(DetInfo::Detector det, int iDevId, float& exposureTime, float& coolingTemp, int& gain, int& readoutSpeed)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Fli,iDevId),
                                        TypeId (TypeId::Id_FliConfig,1) );
  if (xtc ) 
  {    
    if ( xtc->damage.value() != 0 )
      return xtc->damage.value();
    
    const Fli::ConfigV1& fliCfg = *reinterpret_cast<const Fli::ConfigV1*>(xtc->payload());
    exposureTime  = fliCfg.exposureTime     ();
    coolingTemp   = fliCfg.coolingTemp      ();
    gain          = fliCfg.gainIndex        ();
    readoutSpeed  = fliCfg.readoutSpeedIndex();
    
    return 0;
  }
  
  return 2; // no fli config found
}
//
// Configuration includes chargeAmpRange for channels 0-3.
// Values: 0=high gain, 1=medium gain, 2=low gain
//
int getIpimbConfig(DetInfo::Detector det, int iDevId, uint64_t& serialID,
                   int& chargeAmpRange0, int& chargeAmpRange1,
                   int& chargeAmpRange2, int& chargeAmpRange3)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Ipimb,iDevId),
                                        TypeId (TypeId::Id_IpimbConfig,1) );
  if (xtc && xtc->damage.value()==0) {
    const Ipimb::ConfigV1& cfg = *reinterpret_cast<const Ipimb::ConfigV1*>(xtc->payload());
    serialID = cfg.serialID();
    chargeAmpRange0 = cfg.chargeAmpRange() & 0x3;
    chargeAmpRange1 = (cfg.chargeAmpRange() >> 2) & 0x3;
    chargeAmpRange2 = (cfg.chargeAmpRange() >> 4) & 0x3;
    chargeAmpRange3 = (cfg.chargeAmpRange() >> 6) & 0x3;
  }
  return xtc ? xtc->damage.value() : 2;
}

int getEncoderConfig(DetInfo::Detector det, int iDevId)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Encoder,0),
                                        TypeId (TypeId::Id_EncoderConfig,2) );

  if (xtc) 
  {
    if (xtc->damage.value() != 0 )
      return xtc->damage.value();
      
    const Encoder::ConfigV2& cfg = *reinterpret_cast<const Encoder::ConfigV2*>(xtc->payload());
    cfg.dump();
    return 0;
  }

  xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Encoder,0),
                             TypeId (TypeId::Id_EncoderConfig,1) );

  if (xtc)
  {
    if (xtc->damage.value() != 0)
      return xtc->damage.value();

    const Encoder::ConfigV1& cfg = *reinterpret_cast<const Encoder::ConfigV1*>(xtc->payload());
    cfg.dump();
    return 0;
  }

  return 2;  // no encoder config found
}

int getGsc16aiConfig(DetInfo::Detector det, int iDetId, int iDevId,
                     uint16_t&   voltageRange,
                     uint16_t&   firstChan,
                     uint16_t&   lastChan,
                     uint16_t&   inputMode,
                     uint16_t&   triggerMode,
                     uint16_t&   dataFormat,
                     uint16_t&   fps,
                     bool&       autocalibEnable,
                     bool&       timeTagEnable,
                     double&     voltsMin,
                     double&     voltsPerCount)
{
  int rv = 2;
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,iDetId,DetInfo::Gsc16ai,iDevId),
                                        TypeId (TypeId::Id_Gsc16aiConfig,1) );
  if (xtc) {    
    rv = xtc->damage.value();
    if (0 == rv) {
      const Gsc16ai::ConfigV1& gscCfg = *reinterpret_cast<const Gsc16ai::ConfigV1*>(xtc->payload());
      voltageRange = gscCfg.voltageRange();
      firstChan = gscCfg.firstChan();
      lastChan = gscCfg.lastChan();
      inputMode = gscCfg.inputMode();
      triggerMode = gscCfg.triggerMode();
      dataFormat = gscCfg.dataFormat();
      fps = gscCfg.fps();
      autocalibEnable = gscCfg.autocalibEnable();
      timeTagEnable = gscCfg.timeTagEnable();
      switch (gscCfg.voltageRange()) {
        case Pds::Gsc16ai::ConfigV1::VoltageRange_10V:
          voltsMin = -10.0;
          voltsPerCount = 20.0 / 0xffffu;
          break;
        case Pds::Gsc16ai::ConfigV1::VoltageRange_5V:
          voltsMin = -5.0;
          voltsPerCount = 10.0 / 0xffffu;
          break;
        case Pds::Gsc16ai::ConfigV1::VoltageRange_2_5V:
          voltsMin = -2.5;
          voltsPerCount = 5.0 / 0xffffu;
          break;
        default:
          voltsMin = voltsPerCount = 0.0;
          fprintf(stderr, "Error: gsc16ai data voltage range %hd not recognized\n",
                  gscCfg.voltageRange());
          break;
      }
    }
  }
  return (rv);
}

int getTimepixConfig(Pds::DetInfo::Detector det, int iDetId, int iDevId,
                     uint8_t& readoutSpeed, uint8_t& triggerMode, int32_t& timepixSpeed,
                     int32_t& dac0Ikrum, int32_t& dac0Disc, int32_t& dac0Preamp,
                     int32_t& dac0BufAnalogA, int32_t& dac0BufAnalogB, int32_t& dac0Hist,
                     int32_t& dac0ThlFine, int32_t& dac0ThlCourse, int32_t& dac0Vcas,
                     int32_t& dac0Fbk, int32_t& dac0Gnd, int32_t& dac0Ths,
                     int32_t& dac0BiasLvds, int32_t& dac0RefLvds,
                     int32_t& dac1Ikrum, int32_t& dac1Disc, int32_t& dac1Preamp,
                     int32_t& dac1BufAnalogA, int32_t& dac1BufAnalogB, int32_t& dac1Hist,
                     int32_t& dac1ThlFine, int32_t& dac1ThlCourse, int32_t& dac1Vcas,
                     int32_t& dac1Fbk, int32_t& dac1Gnd, int32_t& dac1Ths,
                     int32_t& dac1BiasLvds, int32_t& dac1RefLvds,
                     int32_t& dac2Ikrum, int32_t& dac2Disc, int32_t& dac2Preamp,
                     int32_t& dac2BufAnalogA, int32_t& dac2BufAnalogB, int32_t& dac2Hist,
                     int32_t& dac2ThlFine, int32_t& dac2ThlCourse, int32_t& dac2Vcas,
                     int32_t& dac2Fbk, int32_t& dac2Gnd, int32_t& dac2Ths,
                     int32_t& dac2BiasLvds, int32_t& dac2RefLvds,
                     int32_t& dac3Ikrum, int32_t& dac3Disc, int32_t& dac3Preamp,
                     int32_t& dac3BufAnalogA, int32_t& dac3BufAnalogB, int32_t& dac3Hist,
                     int32_t& dac3ThlFine, int32_t& dac3ThlCourse, int32_t& dac3Vcas,
                     int32_t& dac3Fbk, int32_t& dac3Gnd, int32_t& dac3Ths,
                     int32_t& dac3BiasLvds, int32_t& dac3RefLvds,
                     int32_t& chipCount, int32_t& driverVersion, uint32_t& firmwareVersion,
                     uint32_t& pixelThreshSize, const uint8_t*& pixelThresh,
                     const char*& chip0Name, const char*& chip1Name,
                     const char*& chip2Name, const char*& chip3Name,
                     int32_t& chip0ID, int32_t& chip1ID, int32_t& chip2ID, int32_t& chip3ID)
{
  int rv = 2;
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,iDetId,DetInfo::Timepix,iDevId),
                                        TypeId (TypeId::Id_TimepixConfig,1) );
  if (xtc) {    
    rv = xtc->damage.value();
    if (0 == rv) {
      const Timepix::ConfigV1& timepixCfg = *reinterpret_cast<const Timepix::ConfigV1*>(xtc->payload());

      readoutSpeed = timepixCfg.readoutSpeed();
      triggerMode = timepixCfg.triggerMode();
      timepixSpeed = timepixCfg.shutterTimeout();

      dac0Ikrum = timepixCfg.dac0Ikrum();
      dac0Disc = timepixCfg.dac0Disc();
      dac0Preamp = timepixCfg.dac0Preamp();
      dac0BufAnalogA = timepixCfg.dac0BufAnalogA();
      dac0BufAnalogB = timepixCfg.dac0BufAnalogB();
      dac0Hist = timepixCfg.dac0Hist();
      dac0ThlFine = timepixCfg.dac0ThlFine();
      dac0ThlCourse = timepixCfg.dac0ThlCourse();
      dac0Vcas = timepixCfg.dac0Vcas();
      dac0Fbk = timepixCfg.dac0Fbk();
      dac0Gnd = timepixCfg.dac0Gnd();
      dac0Ths = timepixCfg.dac0Ths();
      dac0BiasLvds = timepixCfg.dac0BiasLvds();
      dac0RefLvds = timepixCfg.dac0RefLvds();

      dac1Ikrum = timepixCfg.dac1Ikrum();
      dac1Disc = timepixCfg.dac1Disc();
      dac1Preamp = timepixCfg.dac1Preamp();
      dac1BufAnalogA = timepixCfg.dac1BufAnalogA();
      dac1BufAnalogB = timepixCfg.dac1BufAnalogB();
      dac1Hist = timepixCfg.dac1Hist();
      dac1ThlFine = timepixCfg.dac1ThlFine();
      dac1ThlCourse = timepixCfg.dac1ThlCourse();
      dac1Vcas = timepixCfg.dac1Vcas();
      dac1Fbk = timepixCfg.dac1Fbk();
      dac1Gnd = timepixCfg.dac1Gnd();
      dac1Ths = timepixCfg.dac1Ths();
      dac1BiasLvds = timepixCfg.dac1BiasLvds();
      dac1RefLvds = timepixCfg.dac1RefLvds();

      dac2Ikrum = timepixCfg.dac2Ikrum();
      dac2Disc = timepixCfg.dac2Disc();
      dac2Preamp = timepixCfg.dac2Preamp();
      dac2BufAnalogA = timepixCfg.dac2BufAnalogA();
      dac2BufAnalogB = timepixCfg.dac2BufAnalogB();
      dac2Hist = timepixCfg.dac2Hist();
      dac2ThlFine = timepixCfg.dac2ThlFine();
      dac2ThlCourse = timepixCfg.dac2ThlCourse();
      dac2Vcas = timepixCfg.dac2Vcas();
      dac2Fbk = timepixCfg.dac2Fbk();
      dac2Gnd = timepixCfg.dac2Gnd();
      dac2Ths = timepixCfg.dac2Ths();
      dac2BiasLvds = timepixCfg.dac2BiasLvds();
      dac2RefLvds = timepixCfg.dac2RefLvds();

      dac3Ikrum = timepixCfg.dac3Ikrum();
      dac3Disc = timepixCfg.dac3Disc();
      dac3Preamp = timepixCfg.dac3Preamp();
      dac3BufAnalogA = timepixCfg.dac3BufAnalogA();
      dac3BufAnalogB = timepixCfg.dac3BufAnalogB();
      dac3Hist = timepixCfg.dac3Hist();
      dac3ThlFine = timepixCfg.dac3ThlFine();
      dac3ThlCourse = timepixCfg.dac3ThlCourse();
      dac3Vcas = timepixCfg.dac3Vcas();
      dac3Fbk = timepixCfg.dac3Fbk();
      dac3Gnd = timepixCfg.dac3Gnd();
      dac3Ths = timepixCfg.dac3Ths();
      dac3BiasLvds = timepixCfg.dac3BiasLvds();
      dac3RefLvds = timepixCfg.dac3RefLvds();

      // ConfigV1 is smaller than ConfigV2, so fill in default values
      chipCount = Timepix::ConfigV1::ChipCount;
      driverVersion = firmwareVersion = pixelThreshSize = 0;
      pixelThresh = (uint8_t *)NULL;
      chip0Name = chip1Name = chip2Name = chip3Name = (char *)NULL;
      chip0ID = chip1ID = chip2ID = chip3ID = 0;
    }
  } else {
    xtc = _estore->lookup_cfg( DetInfo(0,det,iDetId,DetInfo::Timepix,iDevId),
                                        TypeId (TypeId::Id_TimepixConfig,2) );
    if (xtc) {    
      rv = xtc->damage.value();
      if (rv == 0) {
        const Timepix::ConfigV2& timepixCfg = *reinterpret_cast<const Timepix::ConfigV2*>(xtc->payload());

        readoutSpeed = timepixCfg.readoutSpeed();
        triggerMode = timepixCfg.triggerMode();
        timepixSpeed = timepixCfg.timepixSpeed();

        dac0Ikrum = timepixCfg.dac0Ikrum();
        dac0Disc = timepixCfg.dac0Disc();
        dac0Preamp = timepixCfg.dac0Preamp();
        dac0BufAnalogA = timepixCfg.dac0BufAnalogA();
        dac0BufAnalogB = timepixCfg.dac0BufAnalogB();
        dac0Hist = timepixCfg.dac0Hist();
        dac0ThlFine = timepixCfg.dac0ThlFine();
        dac0ThlCourse = timepixCfg.dac0ThlCourse();
        dac0Vcas = timepixCfg.dac0Vcas();
        dac0Fbk = timepixCfg.dac0Fbk();
        dac0Gnd = timepixCfg.dac0Gnd();
        dac0Ths = timepixCfg.dac0Ths();
        dac0BiasLvds = timepixCfg.dac0BiasLvds();
        dac0RefLvds = timepixCfg.dac0RefLvds();

        dac1Ikrum = timepixCfg.dac1Ikrum();
        dac1Disc = timepixCfg.dac1Disc();
        dac1Preamp = timepixCfg.dac1Preamp();
        dac1BufAnalogA = timepixCfg.dac1BufAnalogA();
        dac1BufAnalogB = timepixCfg.dac1BufAnalogB();
        dac1Hist = timepixCfg.dac1Hist();
        dac1ThlFine = timepixCfg.dac1ThlFine();
        dac1ThlCourse = timepixCfg.dac1ThlCourse();
        dac1Vcas = timepixCfg.dac1Vcas();
        dac1Fbk = timepixCfg.dac1Fbk();
        dac1Gnd = timepixCfg.dac1Gnd();
        dac1Ths = timepixCfg.dac1Ths();
        dac1BiasLvds = timepixCfg.dac1BiasLvds();
        dac1RefLvds = timepixCfg.dac1RefLvds();

        dac2Ikrum = timepixCfg.dac2Ikrum();
        dac2Disc = timepixCfg.dac2Disc();
        dac2Preamp = timepixCfg.dac2Preamp();
        dac2BufAnalogA = timepixCfg.dac2BufAnalogA();
        dac2BufAnalogB = timepixCfg.dac2BufAnalogB();
        dac2Hist = timepixCfg.dac2Hist();
        dac2ThlFine = timepixCfg.dac2ThlFine();
        dac2ThlCourse = timepixCfg.dac2ThlCourse();
        dac2Vcas = timepixCfg.dac2Vcas();
        dac2Fbk = timepixCfg.dac2Fbk();
        dac2Gnd = timepixCfg.dac2Gnd();
        dac2Ths = timepixCfg.dac2Ths();
        dac2BiasLvds = timepixCfg.dac2BiasLvds();
        dac2RefLvds = timepixCfg.dac2RefLvds();

        dac3Ikrum = timepixCfg.dac3Ikrum();
        dac3Disc = timepixCfg.dac3Disc();
        dac3Preamp = timepixCfg.dac3Preamp();
        dac3BufAnalogA = timepixCfg.dac3BufAnalogA();
        dac3BufAnalogB = timepixCfg.dac3BufAnalogB();
        dac3Hist = timepixCfg.dac3Hist();
        dac3ThlFine = timepixCfg.dac3ThlFine();
        dac3ThlCourse = timepixCfg.dac3ThlCourse();
        dac3Vcas = timepixCfg.dac3Vcas();
        dac3Fbk = timepixCfg.dac3Fbk();
        dac3Gnd = timepixCfg.dac3Gnd();
        dac3Ths = timepixCfg.dac3Ths();
        dac3BiasLvds = timepixCfg.dac3BiasLvds();
        dac3RefLvds = timepixCfg.dac3RefLvds();

        chipCount = timepixCfg.chipCount();
        driverVersion = timepixCfg.driverVersion();
        firmwareVersion = timepixCfg.firmwareVersion();
        pixelThreshSize = timepixCfg.pixelThreshSize();
        pixelThresh = timepixCfg.pixelThresh();
        chip0Name = timepixCfg.chip0Name();
        chip1Name = timepixCfg.chip1Name();
        chip2Name = timepixCfg.chip2Name();
        chip3Name = timepixCfg.chip3Name();
        chip0ID = timepixCfg.chip0ID();
        chip1ID = timepixCfg.chip1ID();
        chip2ID = timepixCfg.chip2ID();
        chip3ID = timepixCfg.chip3ID();
      }
    }
  }

  return (rv);
}

/*
 * L1Accept data retrieval functions
 */
int getEvrDataNumber()
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,DetInfo::NoDetector,0,
                                                DetInfo::Evr,0),
                                        TypeId (TypeId::Id_EvrData,3) );
  if (xtc && xtc->damage.value()==0)
    return reinterpret_cast<const EvrData::DataV3*>(xtc->payload())->numFifoEvents();
  else
    return 0;
}

int getEvrData( int id, unsigned int& eventCode, unsigned int& fiducial, unsigned int& timeStamp )
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,DetInfo::NoDetector,0,
                                                DetInfo::Evr,0),
                                        TypeId (TypeId::Id_EvrData,3) );
  if (xtc && xtc->damage.value()==0) {
    const EvrData::DataV3::FIFOEvent& fifoEvent = 
      reinterpret_cast<const EvrData::DataV3*>(xtc->payload())->fifoEvent(id);
    eventCode = fifoEvent.EventCode;
    fiducial  = fifoEvent.TimestampHigh;
    timeStamp = fifoEvent.TimestampLow;
  }
  return xtc ? xtc->damage.value() : 2;
}

int getAcqValue(DetInfo info, int channel, double*& time, double*& voltage, double& trigtime)
{
  const Xtc* xtc = _estore->lookup_evt( info, TypeId(TypeId::Id_AcqWaveform,1) );
  if (xtc && (xtc->damage.value()&~(1<<Damage::OutOfOrder))==0) {

    const Acqiris::ConfigV1& acqCfg = 
      *reinterpret_cast<const Acqiris::ConfigV1*>
      (_estore->lookup_cfg( info, TypeId(TypeId::Id_AcqConfig,1) )->payload());
                                                  
    const Acqiris::HorizV1& hcfg = acqCfg.horiz();
    double sampInterval = hcfg.sampInterval();
    Acqiris::DataDescV1* ddesc = reinterpret_cast<Acqiris::DataDescV1*>(xtc->payload());
        
    for (int i=0;i<channel;i++)
      ddesc = ddesc->nextChannel(hcfg);

    const int16_t* data = ddesc->waveform(hcfg);
    data += ddesc->indexFirstPoint();
    const Acqiris::VertV1& vcfg = acqCfg.vert(channel);
    float slope = vcfg.slope();
    float offset = vcfg.offset();
    int nbrSamples = hcfg.nbrSamples();
            
    if (channel >= (int)AcqChannelList.size())
      AcqChannelList.resize(channel+1);

    AcqWaveForm& waveForm = AcqChannelList[channel];
    if ( nbrSamples > (int) waveForm.vfTime.size() )
      { // grow the size of vtTime and vfVoltage at the same time
        waveForm.vfTime   .resize(nbrSamples);
        waveForm.vfVoltage.resize(nbrSamples);
      }
           
    waveForm.vfTrig = ddesc->timestamp(0).pos(); 
    for (int j=0;j<nbrSamples;j++) 
      {
        int16_t swap = (data[j]&0xff<<8) | (data[j]&0xff00>>8);
        waveForm.vfTime[j] = j*sampInterval + waveForm.vfTrig;  //time vector with horPos Offset
        waveForm.vfVoltage[j] = swap*slope-offset;
      }

    time     = &waveForm.vfTime[0];
    voltage  = &waveForm.vfVoltage[0];
    trigtime =  waveForm.vfTrig;

  }

  return xtc ? xtc->damage.value() : 2;
}

int getAcqValue(AcqDetector det, int channel, double*& time, double*& voltage, double& trigtime)
{ return getAcqValue( AcqDetectorIndex( det ), channel, time, voltage, trigtime ); }

int getAcqValue(AcqDetector det, int channel, double*& time, double*& voltage)
{ double trigtime; return getAcqValue(det, channel, time, voltage, trigtime); }

unsigned long long getAcqTime(DetInfo info) 
{
  const Xtc* xtc = _estore->lookup_evt( info, TypeId(TypeId::Id_AcqWaveform,1) );
  if (xtc && (xtc->damage.value()&~(1<<Damage::OutOfOrder))==0) {
    Acqiris::DataDescV1* ddesc = reinterpret_cast<Acqiris::DataDescV1*>(xtc->payload());
    return ddesc->timestamp(0).value();
  }
  return 0;
}

unsigned long long getAcqTime(AcqDetector det) 
{ return getAcqTime( AcqDetectorIndex( det ) ); }

int getFrameValue(FrameDetector det, int& frameWidth, int& frameHeight, unsigned short*& image )
{
  return getFrameValue( FrameDetectorIndex(det), frameWidth, frameHeight, image);
}

int getFrameValue(DetInfo info, int& frameWidth, int& frameHeight, unsigned short*& image )
{
  const Xtc* xtc = _estore->lookup_evt( info, TypeId(TypeId::Id_Frame,1) );
  if (xtc && xtc->damage.value()==0) {
    const Camera::FrameV1& frame = *reinterpret_cast<const Camera::FrameV1*>(xtc->payload());
    frameWidth  = frame.width();
    frameHeight = frame.height();
    image       = (unsigned short*) frame.data();
  }
  return xtc ? xtc->damage.value() : 2;
}

int getFrameValue(BldInfo info, int& frameWidth, int& frameHeight, unsigned short*& image )
{
  const Xtc* xtc = _estore->lookup_evt( info, TypeId(TypeId::Id_Frame,1) );
  if (xtc) {
    if (xtc->damage.value()==0) {
      const Camera::FrameV1& frame = *reinterpret_cast<const Camera::FrameV1*>(xtc->payload());
      frameWidth  = frame.width();
      frameHeight = frame.height();
      image       = (unsigned short*) frame.data();
    }
  }
  else {
    xtc = _estore->lookup_evt( info, TypeId(TypeId::Id_SharedPim,1) );
    if (xtc) {
      if (xtc->damage.value()==0) {
        const BldDataPimV1& pim = *reinterpret_cast<const BldDataPimV1*>(xtc->payload());
        const Camera::FrameV1& frame = pim.frame;
        frameWidth  = frame.width();
        frameHeight = frame.height();
        image       = (unsigned short*) frame.data();
      }
    }
  }
  return xtc ? xtc->damage.value() : 2;
}

int getPrincetonValue(DetInfo::Detector det, int iDevId, unsigned short *& image)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Princeton,iDevId),
                                        TypeId(TypeId::Id_PrincetonFrame,1) );
  if (xtc && xtc->damage.value()==0) {
    const Princeton::FrameV1& princetonFrame = *reinterpret_cast<const Princeton::FrameV1*>(xtc->payload());
    image = (unsigned short*) princetonFrame.data();
  }
  return xtc ? xtc->damage.value() : 2;
}

int getPrincetonTemperature(DetInfo::Detector det, int iDevId, float & temperature)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Princeton,iDevId),
                                        TypeId(TypeId::Id_PrincetonInfo,1) );
  if (xtc && xtc->damage.value()==0) {
    const Princeton::InfoV1& princetonInfo = *reinterpret_cast<Princeton::InfoV1*>(xtc->payload());
    temperature = princetonInfo.temperature();
  }
  return xtc ? xtc->damage.value() : 2;
}

int getFliValue(Pds::DetInfo::Detector det, int iDevId, unsigned short *& image, float& temperature)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Fli,iDevId),
                                        TypeId(TypeId::Id_FliFrame,1) );
  if (xtc && xtc->damage.value()==0) {
    const Fli::FrameV1& fliFrame = *reinterpret_cast<const Fli::FrameV1*>(xtc->payload());
    image       = (unsigned short*) fliFrame.data();
    temperature = fliFrame.temperature();
  }
  return xtc ? xtc->damage.value() : 2;
}

struct OceanOpticsValue
{
  vector<double> vfWavelengths;
  vector<double> vfCounts;
};
static OceanOpticsValue oceanOpticsValue;

int getOceanOpticsValue(Pds::DetInfo::Detector det, int iDevId, int& numPixels, double*& wavelengths, double*& counts)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::OceanOptics,iDevId), TypeId(TypeId::Id_OceanOpticsData,1) );                                         
  if (xtc == NULL)            
    return 1;  
  if (xtc->damage.value()!=0) 
    return xtc->damage.value();
    
  const Xtc* xtcConfig = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::OceanOptics,iDevId), TypeId (TypeId::Id_OceanOpticsConfig,1) );
  if (xtcConfig == NULL)
    return 1;  
  if (xtcConfig->damage.value()!=0)
    return xtcConfig->damage.value();
  
  const OceanOptics::DataV1& oceanOpticsData     = *reinterpret_cast<const OceanOptics::DataV1*>(xtc->payload());
  const OceanOptics::ConfigV1& oceanOpticsConfig = *reinterpret_cast<const OceanOptics::ConfigV1*>(xtcConfig->payload());        
     
  oceanOpticsValue.vfWavelengths.resize(OceanOptics::DataV1::iNumPixels);
  oceanOpticsValue.vfCounts     .resize(OceanOptics::DataV1::iNumPixels);

  for (int iPixel=0; iPixel<OceanOptics::DataV1::iNumPixels; ++iPixel)
  {
    oceanOpticsValue.vfWavelengths[iPixel] = OceanOptics::DataV1::waveLength(oceanOpticsConfig, iPixel);
    oceanOpticsValue.vfCounts     [iPixel] = oceanOpticsData.nonlinerCorrected(oceanOpticsConfig, iPixel);
  }
      
  numPixels   = OceanOptics::DataV1::iNumPixels;
  wavelengths = &oceanOpticsValue.vfWavelengths[0];
  counts      = &oceanOpticsValue.vfCounts     [0];
  return 0;
}

int getIpimbVolts(DetInfo::Detector det, int iDetId, int iDevId,
                  float &channel0, float &channel1, float &channel2, float &channel3)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,iDetId,DetInfo::Ipimb,iDevId),
                                        TypeId(TypeId::Id_IpimbData,2) );
  if (xtc) {
    if (xtc->damage.value()==0) {
      const Ipimb::DataV2& ipimbData = *reinterpret_cast<const Ipimb::DataV2*>(xtc->payload());
      channel0 = ipimbData.channel0Volts();
      channel1 = ipimbData.channel1Volts();
      channel2 = ipimbData.channel2Volts();
      channel3 = ipimbData.channel3Volts();
    }
  }
  else {
    xtc = _estore->lookup_evt( DetInfo(0,det,iDetId,DetInfo::Ipimb,iDevId),
                               TypeId(TypeId::Id_IpimbData,1) );
    if (xtc) {
      if (xtc->damage.value()==0) {
        const Ipimb::DataV1& ipimbData = *reinterpret_cast<const Ipimb::DataV1*>(xtc->payload());
        channel0 = ipimbData.channel0Volts();
        channel1 = ipimbData.channel1Volts();
        channel2 = ipimbData.channel2Volts();
        channel3 = ipimbData.channel3Volts();
      }
    }
  }
  return xtc ? xtc->damage.value() : 2;
}

int getIpimbVolts(DetInfo::Detector det, int iDevId,
                  float &channel0, float &channel1, float &channel2, float &channel3)
{
  return getIpimbVolts(det, 0, iDevId, channel0, channel1, channel2, channel3);
}

int getBldIpimbVolts(BldInfo::Type bldType, float &channel0, float &channel1,
                     float &channel2, float &channel3)
{
  const Xtc* xtc;

  if ( (xtc = _estore->lookup_evt( BldInfo(0,bldType),
                                   TypeId(TypeId::Id_SharedIpimb,0) )) &&
       xtc->damage.value()==0) {
    const BldDataIpimbV0& bldData = *reinterpret_cast<const BldDataIpimbV0*>(xtc->payload());
    const IpimbDataV1& d = bldData.ipimbData;
    channel0 = d.channel0Volts();
    channel1 = d.channel1Volts();
    channel2 = d.channel2Volts();
    channel3 = d.channel3Volts();
  }
  else if ( (xtc = _estore->lookup_evt( BldInfo(0,bldType),
                                        TypeId(TypeId::Id_SharedIpimb,1))) &&
            xtc->damage.value()==0 ) {
    const BldDataIpimbV1& bldData = *reinterpret_cast<const BldDataIpimbV1*>(xtc->payload());
    const IpimbDataV2& d = bldData.ipimbData;
    channel0 = d.channel0Volts();
    channel1 = d.channel1Volts();
    channel2 = d.channel2Volts();
    channel3 = d.channel3Volts();
  }

  return xtc ? xtc->damage.value() : 2;
}

int getBldIpimbConfig(BldInfo::Type bldType, uint64_t& serialID,
                      int& chargeAmpRange0, int& chargeAmpRange1,
                      int& chargeAmpRange2, int& chargeAmpRange3)
{
  const Xtc* xtc;
  if ( (xtc = _estore->lookup_evt( BldInfo(0,bldType),
                                   TypeId(TypeId::Id_SharedIpimb,0) )) &&
       xtc->damage.value()==0 ) {
    const BldDataIpimbV0& bldData = *reinterpret_cast<const BldDataIpimbV0*>(xtc->payload());
    const Ipimb::ConfigV1& bldIpimbConfig = bldData.ipimbConfig;
    serialID = bldIpimbConfig.serialID();
    chargeAmpRange0 = (bldIpimbConfig.chargeAmpRange() >> 0) & 0x3;
    chargeAmpRange1 = (bldIpimbConfig.chargeAmpRange() >> 2) & 0x3;
    chargeAmpRange2 = (bldIpimbConfig.chargeAmpRange() >> 4) & 0x3;
    chargeAmpRange3 = (bldIpimbConfig.chargeAmpRange() >> 6) & 0x3;
  }
  else if ( (xtc = _estore->lookup_evt( BldInfo(0,bldType),
                                        TypeId(TypeId::Id_SharedIpimb,1) )) &&
            xtc->damage.value()==0 ) {
    const BldDataIpimbV1& bldData = *reinterpret_cast<const BldDataIpimbV1*>(xtc->payload());
    const Ipimb::ConfigV2& bldIpimbConfig = bldData.ipimbConfig;
    serialID = bldIpimbConfig.serialID();
    chargeAmpRange0 = (bldIpimbConfig.chargeAmpRange() >>  0) & 0xf;
    chargeAmpRange1 = (bldIpimbConfig.chargeAmpRange() >>  4) & 0xf;
    chargeAmpRange2 = (bldIpimbConfig.chargeAmpRange() >>  8) & 0xf;
    chargeAmpRange3 = (bldIpimbConfig.chargeAmpRange() >> 12) & 0xf;
  }
  return xtc ? xtc->damage.value() : 2;
}

int getBldIpmFexValue   (BldInfo::Type bldType, float* channels, 
                         float& sum, float& xpos, float& ypos)
{
  const Lusi::IpmFexV1* bldIpmFexData = 0;
  const Xtc* xtc;
  if ( (xtc = _estore->lookup_evt( BldInfo(0,bldType),
                                   TypeId(TypeId::Id_SharedIpimb,0) )) &&
       xtc->damage.value()==0) {
    const BldDataIpimbV0& bldData = *reinterpret_cast<const BldDataIpimbV0*>(xtc->payload());
    bldIpmFexData = &bldData.ipmFexData;
  }
  else if ( (xtc = _estore->lookup_evt( BldInfo(0,bldType),
                                   TypeId(TypeId::Id_SharedIpimb,1) )) &&
       xtc->damage.value()==0) {
    const BldDataIpimbV1& bldData = *reinterpret_cast<const BldDataIpimbV1*>(xtc->payload());
    bldIpmFexData = &bldData.ipmFexData;
  }

  if (bldIpmFexData) {
    for(unsigned i=0; i<Lusi::IpmFexConfigV1::NCHANNELS; i++) 
      channels[i] = bldIpmFexData->channel[i];
    sum  = bldIpmFexData->sum;
    xpos = bldIpmFexData->xpos;
    ypos = bldIpmFexData->ypos;
  }
  return xtc ? xtc->damage.value() : 2;
}


int getEncoderCount(DetInfo::Detector det, int iDevId, int& encoderCount, int chan)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Encoder,iDevId),
                                        TypeId(TypeId::Id_EncoderData,2) );
  if (xtc && xtc->damage.value()==0) {
    encoderCount = reinterpret_cast<const Encoder::DataV2*>(xtc->payload())->value(chan);
  }
  else {
    xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Encoder,iDevId),
                               TypeId(TypeId::Id_EncoderData,1) );
    if (xtc && xtc->damage.value()==0) {
      encoderCount = reinterpret_cast<const Encoder::DataV1*>(xtc->payload())->value();
    }
  }
  return xtc ? xtc->damage.value() : 2;
}

int getDiodeFexValue (DetInfo::Detector det, int iDevId, float& value)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Ipimb,iDevId),
                                        TypeId(TypeId::Id_DiodeFex,1) );
  if (xtc && xtc->damage.value()==0) {
    value = reinterpret_cast<const Lusi::DiodeFexV1*>(xtc->payload())->value;
  }
  return xtc ? xtc->damage.value() : 2;
}

int getIpmFexValue   (DetInfo::Detector det, int iDetId, int iDevId, 
                      float* channels, float& sum, float& xpos, float& ypos)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,iDetId,DetInfo::Ipimb,iDevId),
                                        TypeId(TypeId::Id_IpmFex,1) );
  if (xtc && xtc->damage.value()==0) {
    const Lusi::IpmFexV1* p = reinterpret_cast<const Lusi::IpmFexV1*>(xtc->payload());
    for(unsigned i=0; i<Lusi::IpmFexConfigV1::NCHANNELS; i++) 
      channels[i] = p->channel[i];
    sum  = p->sum;
    xpos = p->xpos;
    ypos = p->ypos;
  }
  return xtc ? xtc->damage.value() : 2;
}

int getIpmFexValue   (DetInfo::Detector det, int iDevId, 
                      float* channels, float& sum, float& xpos, float& ypos)
{
  return getIpmFexValue(det, 0, iDevId, channels, sum, xpos, ypos); 
}

int getCspadData  (DetInfo::Detector det, CsPad::ElementIterator& iter)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Cspad,0),
                                        TypeId(TypeId::Id_CspadElement,2) ); // should be 3 at some point
  if (!xtc)
    xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Cspad,0),
                               TypeId(TypeId::Id_CspadElement,1) );

  if (!xtc || xtc->damage.value())
    return 2;

  { const Xtc* cfg = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad,0),
                                          TypeId(TypeId::Id_CspadConfig,1) );
    if (cfg && cfg->damage.value()==0) {
      iter = CsPad::ElementIterator(*reinterpret_cast<CsPad::ConfigV1*>(cfg->payload()),
                                    *xtc);
      return 0;
    }
  }

  { const Xtc* cfg = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad,0),
                                          TypeId(TypeId::Id_CspadConfig,2) );
    if (cfg && cfg->damage.value()==0) {
      iter = CsPad::ElementIterator(*reinterpret_cast<CsPad::ConfigV2*>(cfg->payload()),
                                    *xtc);
      return 0;
    }
  }


  { const Xtc* cfg = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad,0),
                                          TypeId(TypeId::Id_CspadConfig,3) );
    if (cfg && cfg->damage.value()==0) {
      iter = CsPad::ElementIterator(*reinterpret_cast<CsPad::ConfigV3*>(cfg->payload()),
                                    *xtc);
      return 0;
    }
  }

  { const Xtc* cfg = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad,0),
                                          TypeId(TypeId::Id_CspadConfig,4) );
    if (cfg && cfg->damage.value()==0) {
      iter = CsPad::ElementIterator(*reinterpret_cast<CsPad::ConfigV4*>(cfg->payload()),
                                    *xtc);
      return 0;
    }
  }

  return 2;
}

int getCspad2x2Data (DetInfo::Detector det, const CsPad2x2::ElementV1*& elem)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Cspad2x2,0),
                                        TypeId(TypeId::Id_Cspad2x2Element,1) );

  if (!xtc || xtc->damage.value())
    return 2;

  elem = reinterpret_cast<const CsPad2x2::ElementV1*>(xtc->payload());
  return 0;
}



int getCspad2x2Data (DetInfo::Detector det, const CsPad::MiniElementV1*& elem)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Cspad2x2,0),
                                        TypeId(TypeId::Id_Cspad2x2Element,1) );

  if (!xtc || xtc->damage.value())
    return 2;

  elem = reinterpret_cast<const CsPad::MiniElementV1*>(xtc->payload());
  return 0;
}

int getCspad2x2Data (DetInfo::Detector det, CsPad::ElementIterator& iter) {
  return getCspad2x2Data(det, 0, iter);
}

int getCspad2x2Data (DetInfo::Detector det, int iDevId, CsPad::ElementIterator& iter)
{
  unsigned newFlag = 0; 
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Cspad2x2,iDevId),
                                        TypeId(TypeId::Id_Cspad2x2Element,1) );

  if (!xtc || xtc->damage.value())
    return 2;

  { const Xtc* cfg = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad2x2,iDevId),
                                          TypeId(TypeId::Id_CspadConfig,3) );
    if (!(cfg && cfg->damage.value()==0)) {
      cfg = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Cspad2x2,0),
         TypeId(TypeId::Id_Cspad2x2Config,1) );
      newFlag = 1;
    } else {
      //  Temporarily fix a bad configuration parameter    
      reinterpret_cast<uint32_t*>(cfg->payload())[20] = 3;
    }

    if (cfg && cfg->damage.value()==0) {
      
      const Xtc* nxtc = _estore->buffer( DetInfo(0,det,0,DetInfo::Cspad2x2,iDevId), 
                                         TypeId(TypeId::Id_CspadElement,2),
                                         sizeof(CsPad::ElementHeader)+2*sizeof(CsPad::Section));

      Pds::CsPad::ElementHeader* v2   = reinterpret_cast<Pds::CsPad::ElementHeader*>(nxtc->payload());
      Pds::CsPad::MiniElementV1* mini = reinterpret_cast<Pds::CsPad::MiniElementV1*>(xtc->payload());
      memcpy(v2, mini, sizeof(Pds::CsPad::ElementHeader));
      uint16_t* p0 = reinterpret_cast<uint16_t*>(v2+1);
      uint16_t* p1 = p0 + 2*CsPad::ColumnsPerASIC*CsPad::MaxRowsPerASIC;
      const uint16_t* src = reinterpret_cast<const uint16_t*>(&mini->pair[0][0]);
      const uint16_t* end = reinterpret_cast<const uint16_t*>(mini+1);
      while( src < end ) {
        *p0++ = *src++;
        *p1++ = *src++;
      }

      if (newFlag==0) {
  iter = CsPad::ElementIterator(*reinterpret_cast<CsPad::ConfigV3*>(cfg->payload()),
              *nxtc);
      } else {
  CsPad2x2::ConfigV1* tmpCfgV1 = reinterpret_cast<CsPad2x2::ConfigV1*>(cfg->payload());
  const CsPad::ConfigV3 tmpCfgV3(0,
               0,
               tmpCfgV1->inactiveRunMode(),
               tmpCfgV1->activeRunMode(),
               tmpCfgV1->tdi(),
               tmpCfgV1->payloadSize(), 
               tmpCfgV1->badAsicMask(),// 0
               tmpCfgV1->badAsicMask(),// 1
               tmpCfgV1->asicMask(),
               1,
               tmpCfgV1->roiMask(0));
  iter = CsPad::ElementIterator(tmpCfgV3,
              *nxtc);
      }
      return 0;
    }
  }

  return 2;
}

int getFeeGasDet(double* shotEnergy)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,BldInfo::FEEGasDetEnergy),
                                        TypeId (TypeId::Id_FEEGasDetEnergy,
                                                BldDataFEEGasDetEnergy::version) );
  if (xtc && xtc->damage.value()==0) {
    const BldDataFEEGasDetEnergy* pBldFeeGasDetEnergy = 
      reinterpret_cast<const BldDataFEEGasDetEnergy*>(xtc->payload());
    shotEnergy[0] = pBldFeeGasDetEnergy->f_11_ENRC;
    shotEnergy[1] = pBldFeeGasDetEnergy->f_12_ENRC;
    shotEnergy[2] = pBldFeeGasDetEnergy->f_21_ENRC;
    shotEnergy[3] = pBldFeeGasDetEnergy->f_22_ENRC;
  }
  return xtc ? xtc->damage.value() : 2;
}

int getPhaseCavity(double& fitTime1, double& fitTime2,
                   double& charge1,  double& charge2)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,BldInfo::PhaseCavity),
                                        TypeId (TypeId::Id_PhaseCavity,
                                                BldDataPhaseCavity::version) );
  if (xtc && xtc->damage.value()==0) {
    const BldDataPhaseCavity* pBldPhaseCavity = 
      reinterpret_cast<const BldDataPhaseCavity*>(xtc->payload());
    fitTime1 = pBldPhaseCavity->fFitTime1;
    fitTime2 = pBldPhaseCavity->fFitTime2;
    charge1  = pBldPhaseCavity->fCharge1;
    charge2  = pBldPhaseCavity->fCharge2;
  }
  return xtc ? xtc->damage.value() : 2;
}

int getEBeam(double& charge, double& energy, double& posx, double& posy,
             double& angx, double& angy, double& pkcurr)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,BldInfo::EBeam),
                                        TypeId (TypeId::Id_EBeam,
                                                BldDataEBeam::version) );
  if (xtc && xtc->damage.value()==0) {
    const BldDataEBeam* pBldEBeam = 
      reinterpret_cast<const BldDataEBeam*>(xtc->payload());
    charge = pBldEBeam->fEbeamCharge;    /* in nC */ 
    energy = pBldEBeam->fEbeamL3Energy;  /* in MeV */ 
    posx   = pBldEBeam->fEbeamLTUPosX;   /* in mm */ 
    posy   = pBldEBeam->fEbeamLTUPosY;   /* in mm */ 
    angx   = pBldEBeam->fEbeamLTUAngX;   /* in mrad */ 
    angy   = pBldEBeam->fEbeamLTUAngY;   /* in mrad */  
    pkcurr = pBldEBeam->fEbeamPkCurrBC2; /* in Amps */
  }
  return xtc ? xtc->damage.value() : 2;
}

int getEBeam(double& charge, double& energy, double& posx, double& posy,
             double& angx, double& angy)
{
  double pkc; 
  if ( getEBeam(charge, energy, posx, posy, angx, angy, pkc) == 0 )
    return 0;
    
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,BldInfo::EBeam),
                                        TypeId (TypeId::Id_EBeam,0) );
  if (xtc && xtc->damage.value()==0) {
    const BldDataEBeamV0* pBldEBeamV0 = 
      reinterpret_cast<const BldDataEBeamV0*>(xtc->payload());
    charge = pBldEBeamV0->fEbeamCharge;    /* in nC */ 
    energy = pBldEBeamV0->fEbeamL3Energy;  /* in MeV */ 
    posx   = pBldEBeamV0->fEbeamLTUPosX;   /* in mm */ 
    posy   = pBldEBeamV0->fEbeamLTUPosY;   /* in mm */ 
    angx   = pBldEBeamV0->fEbeamLTUAngX;   /* in mrad */ 
    angy   = pBldEBeamV0->fEbeamLTUAngY;   /* in mrad */  
  }
  return xtc ? xtc->damage.value() : 2;
}

int getEBeamDmg(unsigned& dmg)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,BldInfo::EBeam),
                                        TypeId (TypeId::Id_EBeam,
                                                BldDataEBeam::version) );
  if (xtc) {
    const BldDataEBeam* pBldEBeam = 
      reinterpret_cast<const BldDataEBeam*>(xtc->payload());
    dmg = pBldEBeam->uDamageMask;
    return 0;
  }
  else {
    dmg=0;
    return 2;
  }
}

template <int iDbrId> 
static int getEpicsPvValueByType( const EpicsPvHeader& epicsPv1, const void** ppValue, 
                                  int* piDbrype, struct tm* pTmTimeStamp, int* piNanoSec )
{
  const EpicsPvTime<iDbrId>& epicsPv = static_cast<const EpicsPvTime<iDbrId>& >( epicsPv1 );
  *ppValue = &epicsPv.value;
    
  if ( piDbrype != NULL )
    *piDbrype = epicsPv.iDbrType - DBR_TIME_DOUBLE + DBR_DOUBLE; // convert from time type to basic type
            
  if ( piNanoSec != NULL )
    *piNanoSec = epicsPv.stamp.nsec;
    
  if ( pTmTimeStamp != NULL )
    {
      localtime_r( (const time_t*) (void*) &epicsPv.stamp.secPastEpoch, pTmTimeStamp );
      pTmTimeStamp->tm_year += 20; // Epics Epoch starts from 1990, whereas linux time.h Epoch starts from 1970    
    }
    
  return 0;
}

typedef int (*TEpicValueFuncPtr)(const EpicsPvHeader& epicsPv1, const void** ppValue, 
                                 int* piDbrype, struct tm* pTmTimeStamp, int* piNanoSec);
TEpicValueFuncPtr lpfEpicsValueFunc[] = 
  {
    &getEpicsPvValueByType<DBR_STRING>, &getEpicsPvValueByType<DBR_SHORT>,  &getEpicsPvValueByType<DBR_FLOAT>,
    &getEpicsPvValueByType<DBR_ENUM>,   &getEpicsPvValueByType<DBR_CHAR>,   &getEpicsPvValueByType<DBR_LONG>,
    &getEpicsPvValueByType<DBR_DOUBLE>
  };

int getEpicsPvValue( int pvId, const void*& value, int& dbrype, 
                     struct tm& tmTimeStamp, int& nanoSec )
{
  if ( pvId < 0 || pvId >= (int) _estore->epics().size() )
    { printf( "getEpicsPvConfig(): PV Id (%d) is invalid.\n", pvId ); return 1; }    
  if ( _estore->epics()[pvId] == 0 ) return 2;// Data has never been processed or read from xtc    

  const EpicsPvHeader& epicsPv = *_estore->epics()[ pvId ];    
    
  static const int iBasicType = epicsPv.iDbrType - DBR_TIME_DOUBLE + DBR_DOUBLE;    
  if ( iBasicType < 0 || iBasicType > DBR_DOUBLE ) // Not a valid PV type
    { printf( "getEpicsPvValue(): Data type for PV Id %d is not valid.\n", pvId ); return 3; }     
    
  return ( *lpfEpicsValueFunc[ iBasicType ] ) ( epicsPv, &value, &dbrype, &tmTimeStamp, &nanoSec );
}

int getPvInt(const char* pvName, int& value)
{
  int iPvId = _estore->epics_index(pvName);
  if (iPvId < 0) return 1;
    
  const void* pValue;
  int iDbrType;
  struct tm tmTimeStamp;
  int iNanoSec;
  int iFail = getEpicsPvValue( iPvId, pValue, iDbrType, tmTimeStamp, iNanoSec );
  if ( iFail != 0 ) return 2; // getEpicsPvValue() Failed
    
  switch (iDbrType)
    {
    case DBR_SHORT:
      value = (int) *(short int*) pValue;
      break;
    case DBR_LONG:
      value = *(int*) pValue;
      break;
    default:
      printf( "getPvInt(%s): Not an integer value PV.\n", pvName );
      return 3; // Unsupported iDbrType
    }    

  return 0;
}

int getPvFloat(const char* pvName, float& value)
{
  int iPvId = _estore->epics_index(pvName);
  if (iPvId < 0) return 1;
    
  const void* pValue;
  int iDbrType;
  struct tm tmTimeStamp;
  int iNanoSec;
  int iFail = getEpicsPvValue( iPvId, pValue, iDbrType, tmTimeStamp, iNanoSec );
  if ( iFail != 0 ) return 2; // getEpicsPvValue() Failed
    
  switch (iDbrType)
    {
    case DBR_FLOAT:
      value = *(float*) pValue;
      break;
    case DBR_DOUBLE:
      value = (float) *(double*) pValue;
      break;
    default:
      printf( "getPvFloat(%s): Not an floating point value PV.\n", pvName );    
      return 3; // Unsupported iDbrType
    }    
    
  return 0;    
}

int getPvString(const char* pvName, char*& value)
{
  int iPvId = _estore->epics_index(pvName);
  if (iPvId < 0) return 1;
    
  const void* pValue;
  int iDbrType;
  struct tm tmTimeStamp;
  int iNanoSec;
  int iFail = getEpicsPvValue( iPvId, pValue, iDbrType, tmTimeStamp, iNanoSec );
  if ( iFail != 0 ) return 2; // getEpicsPvValue() Failed
    
  switch (iDbrType)
    {
    case DBR_STRING:
      value = (char *) pValue;
      break;
    default:
      printf( "getPvString(%s): Not an string value PV.\n", pvName );
      return 3; // Unsupported iDbrType
    }    

  return 0;
}

int getPvConfig   (const char* pvName, const char*& pvDescription, double& updateInterval)
{
  int iPvId = _estore->epics_index(pvName);
  if (iPvId < 0) return 1;
  
  return _estore->getPvConfig(iPvId, pvDescription, updateInterval);
}

static const DetInfo::Detector pnccdDets[] = { DetInfo::Camp, DetInfo::SxrEndstation };
static const unsigned npnccdDets =  sizeof(pnccdDets)/sizeof(DetInfo::Detector);

int getPnCcdConfig  ( int deviceId, const PNCCD::ConfigV1*& c )
{
  const Xtc* xtc = 0;
  for(unsigned idet=0; idet<npnccdDets; idet++)
    if ((xtc = _estore->lookup_cfg( DetInfo(0,pnccdDets[idet],0,DetInfo::pnCCD,deviceId),
                                    TypeId (TypeId::Id_pnCCDconfig,1) )))
      break;
        
  if (xtc && xtc->damage.value()==0) {
    c = reinterpret_cast<const PNCCD::ConfigV1*>(xtc->payload());
  }
  return xtc ? xtc->damage.value() : 2;
}

int getPnCcdConfig  ( int deviceId, const PNCCD::ConfigV2*& c )
{
  const Xtc* xtc = 0;
  for(unsigned idet=0; idet<npnccdDets; idet++)
    if ((xtc = _estore->lookup_cfg( DetInfo(0,pnccdDets[idet],0,DetInfo::pnCCD,deviceId),
                                    TypeId (TypeId::Id_pnCCDconfig,2) )))
      break;
        
  if (xtc && xtc->damage.value()==0) {
    c = reinterpret_cast<const PNCCD::ConfigV2*>(xtc->payload());
  }
  return xtc ? xtc->damage.value() : 2;
}

int getPnCcdRaw  ( int deviceId, const PNCCD::FrameV1*& frame )
{
  const Xtc* xtc = 0;
  for(unsigned idet=0; idet<npnccdDets; idet++)
    if ((xtc = _estore->lookup_evt( DetInfo(0,pnccdDets[idet],0,DetInfo::pnCCD,deviceId),
                                    TypeId (TypeId::Id_pnCCDframe,1) )))
      break;
        
  if (xtc && xtc->damage.value()==0) {
    frame = reinterpret_cast<const PNCCD::FrameV1*>(xtc->payload());
  }
  return xtc ? xtc->damage.value() : 2;
}

int getPnCcdValue( int deviceId, unsigned char*& image, int& width, int& height )
{
  const Xtc* xtc = 0;
  unsigned idet=0;
  while(idet<npnccdDets) {
    if ((xtc = _estore->lookup_evt( DetInfo(0,pnccdDets[idet],0,DetInfo::pnCCD,deviceId),
                                    TypeId (TypeId::Id_pnCCDframe,1) )))
      break;
    idet++;
  }
        
  if (xtc && xtc->damage.value()==0) {

    const PNCCD::FrameV1  *pFrame0,*pFrame1,*pFrame2,*pFrame3;

    const PNCCD::ConfigV1*  config = 0;
    if ( getPnCcdConfig(deviceId, config)!=0 ) {
      const PNCCD::ConfigV2*  config2 = 0;
      if ( getPnCcdConfig(deviceId, config2)!=0 ) {
        return 2;
      }
      else {
        pFrame0 =  reinterpret_cast<const PNCCD::FrameV1*>(xtc->payload());
        pFrame1 = pFrame0->next(*config2);
        pFrame2 = pFrame1->next(*config2);
        pFrame3 = pFrame2->next(*config2);  
      }
    }
    else {
      pFrame0 =  reinterpret_cast<const PNCCD::FrameV1*>(xtc->payload());
      pFrame1 = pFrame0->next(*config);
      pFrame2 = pFrame1->next(*config);
      pFrame3 = pFrame2->next(*config);  
    }

    uint8_t* imagePnCcd   = (uint8_t*) _estore->buffer(DetInfo(0,pnccdDets[idet],0,DetInfo::pnCCD,deviceId),
                                                       TypeId (TypeId::Id_pnCCDframe,1),
                                                       iPnCcdImageSize)->payload();

    uint16_t* pLineDst     = (uint16_t*) imagePnCcd;
    uint16_t* pLineSrc0    = (uint16_t*) pFrame0->data();
    uint16_t* pLineSrc1    = (uint16_t*) pFrame3->data();
    for ( int iY = 0; iY < iPnCcdHeightPerLink; iY++ )
      {
        for ( int iX = 0; iX < iPnCcdWidthPerLink; iX++ )
          *pLineDst++ = *pLineSrc0++;
        for ( int iX = 0; iX < iPnCcdWidthPerLink; iX++ )
          *pLineDst++ = *pLineSrc1++;
      }
    
    pLineSrc0 = (uint16_t*) pFrame1->data()+iPnCcdHeightPerLink*iPnCcdWidthPerLink;
    pLineSrc1 = (uint16_t*) pFrame2->data()+iPnCcdHeightPerLink*iPnCcdWidthPerLink;
    for ( int iY = 0; iY < iPnCcdHeightPerLink; iY++ )
      {
        for ( int iX = 0; iX < iPnCcdWidthPerLink; iX++ )
          *pLineDst++ = *--pLineSrc0;
        for ( int iX = 0; iX < iPnCcdWidthPerLink; iX++ )
          *pLineDst++ = *--pLineSrc1;
      }
    
    image   = imagePnCcd;
    width   = iPnCcdWidth;
    height  = iPnCcdHeight;
  }
  return xtc ? xtc->damage.value() : 2;
}

//
// getGsc16aiValue - get value of one Gsc16ai ADC channel
//
// Example usage:
//
//   uint16_t channelValue;
//   double channelVoltage;
//   int channel = 0;
//   int fail = getGsc16aiValue(Pds::DetInfo::XppEndstation,0,0,channelValue,channelVoltage,channel);
//
// Returns: Zero on success, nonzero on error.
//
int getGsc16aiValue(DetInfo::Detector det, int iDetId, int iDevId,
                   uint16_t& channelValue, double& channelVoltage, int chan)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,iDetId,DetInfo::Gsc16ai,iDevId),
                                        TypeId(TypeId::Id_Gsc16aiData,1) );
  if (xtc && xtc->damage.value()==0) {
      uint16_t  voltageRange, firstChan, lastChan, inputMode,
                triggerMode, dataFormat, fps;
      bool      autocalibEnable, timeTagEnable;
      double    voltsMin, voltsPerCount;

      int fail = getGsc16aiConfig(det, iDetId, iDevId, voltageRange, firstChan, lastChan,
                                  inputMode, triggerMode, dataFormat, fps,
                                  autocalibEnable, timeTagEnable, voltsMin, voltsPerCount);
      if (fail) {
        // Error: failed to read configuration
        return (2);
      } else if (chan < firstChan || chan > lastChan) {
        // Error: requested channel is outside the configured range
        return (2);
      } else {
        channelValue = reinterpret_cast<const Gsc16ai::DataV1*>
                        (xtc->payload())->channelValue(chan - firstChan);
        // convert raw value to voltage
        if (dataFormat == Pds::Gsc16ai::ConfigV1::DataFormat_OffsetBinary) {
          // offset binary data format
          channelVoltage = voltsMin + (voltsPerCount * channelValue);
        } else {
          // two's complement data format
          channelVoltage = voltsPerCount * (int16_t)channelValue;
        }
      }
  }
  return xtc ? xtc->damage.value() : 2;
}

int getGsc16aiTimestamp(Pds::DetInfo::Detector det, int iDetId, int iDevId,
                   uint16_t& ts0, uint16_t& ts1, uint16_t& ts2)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,iDetId,DetInfo::Gsc16ai,iDevId),
                                        TypeId(TypeId::Id_Gsc16aiData,1) );
  if (xtc && xtc->damage.value()==0) {
    ts0 = reinterpret_cast<const Gsc16ai::DataV1*>(xtc->payload())->timestamp(0);
    ts1 = reinterpret_cast<const Gsc16ai::DataV1*>(xtc->payload())->timestamp(1);
    ts2 = reinterpret_cast<const Gsc16ai::DataV1*>(xtc->payload())->timestamp(2);
  }
  return xtc ? xtc->damage.value() : 2;
}

/*
 * getTimepixValue - retrieve Timepix data
 *
 * This routine supports both DataV1 and DataV2 Timepix formats and
 * automatically shuffles the data if required.
 *
 * INPUT parameters:
 *
 *   det - Pds::DetInfo::Detector enum
 *   detId - detector ID
 *   devId - Timepix device ID
 *
 * OUTPUT parameters:
 *
 *   timestamp - 32-bit hardware timestamp, 10 usec units.
 *   frameCounter - 16-bit hardware frame counter.
 *   lostRows - if greater than 0, indicates data loss.
 *   frameWidth - pixels per row
 *   frameHeight - pixels per column
 *   image - frameWidth x frameHeight pixel array
 *
 * RETURNS: The xtc damage value, or 2 if no matching xtc is found.
 */

int getTimepixValue(DetInfo::Detector det, int iDetId, int iDevId,
                      uint32_t& timestamp, uint16_t& frameCounter, uint16_t& lostRows,
                      int& frameWidth, int& frameHeight, unsigned short*& image)
{
  const Timepix::DataV2 *dataV2 = 0;

  DetInfo info(0,det,iDetId,DetInfo::Timepix,iDevId);
  const Xtc* xtc = _estore->lookup_evt(info, TypeId(TypeId::Id_TimepixData,1));
  if (xtc) {
    // DataV1 found...
    const Timepix::DataV1 *dataV1 = reinterpret_cast<const Timepix::DataV1*> (xtc->payload());
    // DataV1: shuffle the image by converting to DataV2
    Xtc* bxtc = (Xtc *)_estore->buffer(info,
                                       TypeId(TypeId::Id_TimepixData,2),
                                       sizeof(Pds::Timepix::DataV2) + dataV1->data_size());
    dataV2 = new (reinterpret_cast<char*>(bxtc->payload())) Pds::Timepix::DataV2(*dataV1);
  } else {
    xtc = _estore->lookup_evt(info, TypeId(TypeId::Id_TimepixData,2));
    if (xtc) {
      // DataV2 found...
      dataV2 = reinterpret_cast<const Timepix::DataV2*> (xtc->payload());
    } else {
      return 2;
    }
  }
  // fill in output parameters
  timestamp = dataV2->timestamp();
  frameCounter = dataV2->frameCounter();
  lostRows = dataV2->lostRows();
  frameWidth = dataV2->width();
  frameHeight = dataV2->height();
  image = (unsigned short *) dataV2->data();

  return xtc->damage.value();
}

/*
 * Control data retrieval functions
 */
static string GenerateCtrlPvHashKey( const char* pvName, int arrayIndex)
{  
  if ( arrayIndex == -1 ) arrayIndex = 0;
  std::stringstream sstream; sstream << arrayIndex;
  return string(pvName) + "[" + sstream.str() + "]";
}

int getControlPvNumber()
{
  return _estore->control_data().npvControls();
}

int getControlPvName( int pvId, const char*& pvName, int& arrayIndex )
{
  const ControlData::ConfigV1& config = _estore->control_data();
  if ( pvId < 0 || pvId >= (int) config.npvControls() ) // no such pvId
    return 2;
    
  const ControlData::PVControl& pvControlCur = config.pvControl(pvId);
  pvName = pvControlCur.name();

  if (pvControlCur.array())
    arrayIndex = pvControlCur.index();
  else
    arrayIndex = 0;
    
  return 0;
}

int getControlValue(const char* pvName, int arrayIndex, double& value )
{
  int pvIndex = _estore->control_index(pvName, arrayIndex);
  if (pvIndex<0)
    return 1;

  const ControlData::PVControl& pvControlCur = _estore->control_data().pvControl(pvIndex);
  value = pvControlCur.value();
    
  return 0;
}

int getMonitorPvNumber()
{
  return _estore->control_data().npvMonitors();
}

int getMonitorPvName( int pvId, const char*& pvName, int& arrayIndex )
{
  const ControlData::ConfigV1& config = _estore->control_data();
  if ( pvId < 0 || pvId >= (int) config.npvMonitors() ) // no such pvId
    return 2;
    
  const ControlData::PVMonitor& pvMonitorCur = config.pvMonitor(pvId);
  pvName = pvMonitorCur.name();

  if (pvMonitorCur.array())
    arrayIndex = pvMonitorCur.index();
  else
    arrayIndex = 0;
    
  return 0;
}

int getMonitorValue(const char* pvName, int arrayIndex, double& hilimit, double& lolimit )
{
  int pvIndex = _estore->control_index(pvName, arrayIndex);
  if (pvIndex<0)
    return 1;

  const ControlData::PVMonitor& pvMonitorCur = _estore->control_data().pvMonitor(pvIndex);
          
  hilimit = pvMonitorCur.hiValue();
  lolimit = pvMonitorCur.loValue();
    
  return 0;
}


void usage(char* progname) 
{
  fprintf(stderr,"Usage: %s -f <filename> | -l <filename_list> | -r <run_file_prefix>\n", progname);
  fprintf(stderr,
    "     [-h]\n"
    "     [-c <caliblist>] [-s <skipevts>]\n"
    "     [-n <maxevts>]\n"
    "     [-O <reorder file>]\n"
    "     [-o <output xtc file>]\n"
    "     [-S] (split event recovery)\n"
    "     [-L] (live file read)\n"
    "     [-d <debug level>]\n"
    "     [-j <jumpToEvent>]\n"
    "     [-y <jumpToCalibCycle>]\n"
    "     [-t <jumpToTime>]\n"
    "     [-u <jumpToNextFiducial>[,<searchFromEvent>]]\n"
    "     [-e <eventListFile>]\n");
  fprintf(stderr,
    "  * The -l and -c arguments require files with a list of files in them.\n"
    "  * The -r argument accepts the format of <path>/eXX-rXXXX , or <path>/eXX-rXXXX-sXX-cXX.xtc\n"
    "  * The -e argument requires a list of events. The file format is as follows:\n"
    "      <event1> <event2> # Events are separated by space, \',\' or newlines\n"
    "      <event1>-<event2> # Include all events between <event1> and <event2>\n"
    "      C<calibCycle#> <event1> <event2> # Move to some CalibCycle, and read specified events\n");
}

void makeoutfilename(char* filename, char* outfilename) 
{
  unsigned start= 0;
  unsigned last = strlen(filename) - 1;
  unsigned end  = last;
  for (unsigned i=0;i<last;i++) 
  {
    if (filename[i]=='/') start=i+1;
    if (filename[i]=='.') end=i-1;
  }
    
  strncpy(outfilename,filename+start,end-start+1);  
  strncpy(outfilename+end-start+1,".root",6);
}

static void dump(Dgram* dg, unsigned int uCurCalib, int iSlice, int64_t i64Offset)
{
  char buff[128];
  time_t t = dg->seq.clock().seconds();
  strftime(buff,128,"%Z %a %F %T",localtime(&t));
  printf("<%d> %s.%09u fid 0x%05x vec %03d %s extent 0x%x damage 0x%x\n",         
         iSlice,
         buff,
         dg->seq.clock().nanoseconds(),
         dg->seq.stamp().fiducials(),
         dg->seq.stamp().vector(),
         TransitionId::name(dg->seq.service()),
         dg->xtc.extent, dg->xtc.damage.value());
}

static void dump(Dgram* dg, unsigned int uCurCalib, unsigned int uEventCalibBase, unsigned int uCurEvent, int iSlice, int64_t i64Offset)
{
  char buff[128];
  time_t t = dg->seq.clock().seconds();
  strftime(buff,128,"%H:%M:%S",localtime(&t));
  printf("<%d> %s.%09u fid 0x%05x vec %03d %s extent 0x%x damage 0x%x calib %d event %02d (global %02d)\n",
         iSlice,
         buff,
         dg->seq.clock().nanoseconds(),
         dg->seq.stamp().fiducials(),
         dg->seq.stamp().vector(),
         TransitionId::name(dg->seq.service()),
         dg->xtc.extent, dg->xtc.damage.value(), 
         uCurCalib, uCurEvent - uEventCalibBase + 1,
         uCurEvent);
}

struct EventRange
{
  int calib;
  int event1;
  int event2;
  string strEvent1;
  string strEvent2;
  EventRange(int calibA, int event1A, int event2A, const string& strEvent1A, const string& strEvent2A) : 
    calib(calibA), event1(event1A), event2(event2A), strEvent1(strEvent1A), strEvent2(strEvent2A) {}
};
typedef vector<EventRange> TEventRangeList;

struct EventNo
{
  int calib;
  int event;
  EventNo(int calib, int event) : calib(calib), event(event) {}
};
typedef vector<EventNo> TEventNoList;

int convertEventRangeToNo(XtcRun& run, const TEventRangeList& lEventRange, TEventNoList& lEventNo);

static SplitEventQ* _split;
static XtcWriter*   _writer;

static bool _record;

void event(Dgram& dg) 
{
  _record = false;
  event();
  if (_record && _writer)
    _writer->insert(dg);
}

void record() { _record = true; }

void anarun(XtcRun& run, unsigned &maxevt, unsigned &skip, const char* reorder_file,
            unsigned jump, unsigned calib, const TEventRangeList& lEventRange, 
            char* sTime, uint32_t uFiducialSearch, int iFidFromEvent, int iDebugLevel)
{
  run.init();

  _estore = new EventStore(reorder_file, iDebugLevel);

  Result r = OK;
  unsigned nevent = 1;
  unsigned nprint = 1;
  unsigned damage;
  unsigned damagemask = 0;
  unsigned ndamage = 0;
  bool     bJumpedToEvent  = false; 
  unsigned numProcessedEvents = 0;
  
  TEventNoList lEventNo;
  convertEventRangeToNo(run, lEventRange, lEventNo);
  unsigned  uNextEventNo    = 0;
  bool      bLoadNextEvent  = ( lEventNo.size() != 0 );

  if (uFiducialSearch != (uint32_t) -1)
  {
    int   iCalib = -1, iEvent = -1;
    int   iError = run.findNextFiducial(uFiducialSearch, iFidFromEvent, iCalib, iEvent);
    
    if (iError != 0)
      printf("Cannot find event with the fiducial 0x%x , after global event# %d\n", uFiducialSearch, iFidFromEvent);
    else
    {      
      calib = iCalib;
      jump  = iEvent;
      printf("Going to event with fiducial 0x%x after global event# %d : Calib# %d Event# %d\n",
        uFiducialSearch, iFidFromEvent,
        calib, jump );
    }    
  }
  else if (sTime != NULL)
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
      calib = iCalib;
      jump  = iEvent;
      printf("Going to event with time %s%s Calib# %d Event# %d\n",
        sTime, (bExactMatch? " [exact]":""),
        calib, jump );
    }
  }
  
  if ( jump < 1 && calib < 1 && bLoadNextEvent )
  {
    calib = lEventNo[uNextEventNo].calib;
    jump  = lEventNo[uNextEventNo].event;
    
    ++uNextEventNo;
  }
  
  unsigned int uCurCalib        = 0;
  unsigned int uEventCalibBase  = 0;
  
  while (maxevt > 0 && r == OK) 
  {  
    Dgram*  dg        = NULL;
    int     iSlice    = -1;
    int64_t i64Offset = -1;
    
    r = run.next(dg, &iSlice, &i64Offset);
    if (r == Error || r==End)
      break;

    if (dg->seq.service()!=TransitionId::L1Accept) 
      dump(dg, uCurCalib, iSlice, i64Offset);
    else if (skip) {
      skip--;
      continue;
    }
    else if (nevent%nprint == 0) {
      dump(dg, uCurCalib, uEventCalibBase, nevent, iSlice, i64Offset);
      if (nevent==10*nprint)//!!debug
        nprint *= 10;
    }

    if (_split->cache(dg))
      continue;

    if (dg->seq.service()==TransitionId::EndCalibCycle) 
      {
        for(std::list<Dgram*>::iterator it=_split->queue().begin();
            it!=_split->queue().end(); it++) 
          {
            _estore->processDg(*it);

            damage = (*it)->xtc.damage.value();
            if (damage)
              {
                ndamage++;
                damagemask |= damage;
              }
            
            --maxevt;
            ++nevent;
            
            event(* *it);
          }
        _split->clear();
      }

    _estore->processDg(dg);

    damage = dg->xtc.damage.value();
    if (damage)
    {
        ndamage++;
        damagemask |= damage;
    }

    if (dg->seq.service() != TransitionId::L1Accept && _writer)
      _writer->insert(*dg);

    if (dg->seq.service() == TransitionId::L1Accept)
    {      
      --maxevt;
      
      event(*dg);
      
      ++nevent;      
      ++numProcessedEvents;
  
      if ( bLoadNextEvent )
      {
        if (uNextEventNo >= lEventNo.size())
          break;
        
        //printf( "Loading next event %u from list. prev c %u j %u ",
        //  uNextEventNo, calib, jump ); // !! debug
          
        calib = lEventNo[uNextEventNo].calib;
        jump  = lEventNo[uNextEventNo].event;

        //printf( "next c %u j %u calibCur %u\n", calib, jump, uCurCalib ); //!! debug
        
        if ( calib != uCurCalib )
        {
          int iEventNumAfterJump;
          int iError = run.jump(calib, 0, iEventNumAfterJump);
          if ( iError == 0 )
          {
            nevent          = iEventNumAfterJump;
            uCurCalib       = calib - 1;
            uEventCalibBase = nevent;
            bJumpedToEvent  = false;
          }              
        }
        else
        {
          int iEventNumAfterJump;
          int iError = run.jump(calib, jump, iEventNumAfterJump);
          if ( iError == 0 )
            nevent = iEventNumAfterJump;
        }
        
        _split->clear();
        ++uNextEventNo;
      }         
    }
    else if (dg->seq.service() == TransitionId::Configure)
    {
      if (!jobbegun)
        {
          beginjob();
          jobbegun = 1;
        }
    }
    else if (dg->seq.service() == TransitionId::BeginRun)
    {
      _runnumber = dg->env.value();
      if (_runnumber==0) 
        _runnumber = run.run_number();
        
      bJumpedToEvent = false;
      beginrun();
      
      if ( calib > 1 )
      {
        int iEventNumAfterJump;
        int iError = run.jump(calib, 0, iEventNumAfterJump);
        if ( iError == 0 )
        {
          nevent          = iEventNumAfterJump;
          uCurCalib       = calib - 1;
          uEventCalibBase = nevent;
        }
        _split->clear();
      }
      else if ( calib == 0 )
        calib = 1;
    }    
    else if (dg->seq.service() == TransitionId::BeginCalibCycle)
    {
      ++uCurCalib;
      uEventCalibBase = nevent;
      begincalib();
    }
    else if (dg->seq.service() == TransitionId::EndCalibCycle)
    {
      endcalib();
    }
    else if (dg->seq.service() == TransitionId::Enable)
    {
      if ( jump > 1 && !bJumpedToEvent )
      {            
        int iEventNumAfterJump;
        int iError = run.jump(calib, jump, iEventNumAfterJump);
        if ( iError == 0 )
          nevent = iEventNumAfterJump;
          
        _split->clear();
        bJumpedToEvent = true;
      }       
    } // if (dg->seq.service() == TransitionId::Enable)
      
  }
  
  endrun();
  printf("Processed %d events, %d damaged, with damage mask 0x%x.\n", 
    numProcessedEvents, ndamage, damagemask);

  delete _estore;
}

list<string> calib_files;

int findTimeString(char* sLine, vector<string>& lTimeString)
{
  do
  {
    char* pQuote1 = strchr(sLine,'\"');
    if (pQuote1 == NULL) return 0;
    
    char* pQuote2 = strchr(pQuote1+1,'\"');
    if (pQuote1 == NULL) 
    {
      printf("Unmatched quote in string %s\n", sLine);
      return 0;
    }
    
    lTimeString.push_back( string(pQuote1+1, pQuote2) );
    std::fill(pQuote1, pQuote2+1, '*');
  }
  while (true);
  return 0;
}

int readEventList(const char* evtLstFn, TEventRangeList& lEventRange)
{
  FILE *fList = fopen(evtLstFn, "r");
  if (fList == NULL)
  {
    printf("readEventList(): Failed to open Event List file %s\n", evtLstFn);
    return 1;
  }
  
  lEventRange.clear();
  
  int uCurCalib = 1;
  while (!feof(fList))  
  {
    char sLine[256] = "";
    fgets(sLine, sizeof(sLine)-1, fList);
    
    char* pComment = strchr( sLine, '#' );
    if (pComment != NULL) *pComment = 0;
    
    vector<string> lTimeString;
    findTimeString(sLine, lTimeString);
    
    int iTimeStringNo = 0;
        
    char* pScan1 = NULL;
    for (char* sLineScan = sLine;;sLineScan = NULL)
    {                  
      char* sEventList = strtok_r(sLineScan, ",; \r\n", &pScan1);      
      if (sEventList == NULL) break;
                  
      bool bFoundDash = (strchr(sEventList, '-')  != NULL) ;
      
      int iEventNo1 = -1, iEventNo2 = -1;
      string strEvent1, strEvent2;
      char* pScan2 = NULL;
      for (char* sEventScan = sEventList;;sEventScan = NULL)
      {
        char* sEvent = strtok_r(sEventScan, "-", &pScan2);
        if (sEvent == NULL) break;
        
        if (sEvent[0] == '*')
        {
          if (iTimeStringNo >= (int) lTimeString.size())
          {
            printf("readEventList(): Find non-indexed string mark %s\n", sEvent);
            continue;
          }
                      
          if (strEvent1.empty())
            strEvent1 = lTimeString[iTimeStringNo];
          else
            strEvent2 = lTimeString[iTimeStringNo];
            
          ++iTimeStringNo;          
        }
        else if (sEvent[0] == 'C' || sEvent[0] == 'c')
        {
          int iNewCalib = strtol(sEvent+1, NULL, 0);
          if (iNewCalib > 0) uCurCalib = iNewCalib;
        }
        else
        {
          int iNewEventNo = strtol(sEvent, NULL, 0);
          if (iNewEventNo > 0)
          {
            if (iEventNo1 == -1)
              iEventNo1 = iNewEventNo;
            else
              iEventNo2 = iNewEventNo;
          } // if (iNewEventNo > 0)          
        } // if (sEvent[0] == 'C' || sEvent[0] == 'c')
      } // for (char* sEventScan =sEventList;;sEventScan = NULL)
      
      if (iEventNo1 == -1 && strEvent1.empty()) continue;
            
      if (bFoundDash && iEventNo1 > 0 && iEventNo2 < 0)
        iEventNo2 = 0; // special value, meaning the last event of current calib
        
      //printf("[%d] calib %d event1 %d event2 %d\n", lEventRange.size(), uCurCalib, iEventNo1, iEventNo2);//!!debug
      lEventRange.push_back( EventRange(uCurCalib, iEventNo1, iEventNo2, strEvent1, strEvent2) );
    } // for (char* sLineScan = sLine;;sLineScan = NULL)    
  } // while (!feof(fList)) 
    
  fclose(fList);
  
  return 0;
}

int convertEventRangeToNo(XtcRun& run, const TEventRangeList& lEventRange, TEventNoList& lEventNo)
{
  lEventNo.clear();
  
  int iRange = 0;
  for (TEventRangeList::const_iterator
    itEventRange = lEventRange.begin();
    itEventRange != lEventRange.end();
    ++itEventRange, ++iRange)
  {
    if (itEventRange->event1 > 0)
    {
      if (itEventRange->calib <= 0)
      {
        printf("convertEventRangeToNo(): [%d] Invalid calib %d\n", iRange, itEventRange->calib);
        continue;
      }
      
      if (itEventRange->event2 < 0)
        lEventNo.push_back(EventNo(itEventRange->calib, itEventRange->event1));
      else if (itEventRange->event2 > 0)
        for (int iEventNo = itEventRange->event1; iEventNo <= itEventRange->event2; ++iEventNo)
          lEventNo.push_back(EventNo(itEventRange->calib, iEventNo));
      else 
      { // Remaining case: itEventRange->event2 == 0
        int iNumEvent = -1;
        int iError = run.numEventInCalib(itEventRange->calib, iNumEvent);
        if (iError != 0)
        {
          printf("convertEventRangeToNo(): [%d] Quert Event# in Calib# %d failed\n", iRange, itEventRange->calib);
          continue;
        }
                
        for (int iEventNo = itEventRange->event1; iEventNo <= iNumEvent; ++iEventNo)
          lEventNo.push_back(EventNo(itEventRange->calib, iEventNo));    
      }
    }
    else 
    {
      bool  bExactMatch = false;
      bool  bOvertime   = false;
      int   iCalib1     = -1;
      int   iEvent1     = -1;
      int iError = run.findTime(itEventRange->strEvent1.c_str(), iCalib1, iEvent1, bExactMatch, bOvertime);      
      if (iError != 0)
      {
        if (bOvertime)
          printf("convertEventRangeToNo(): [%d] Time %s is later than the last event\n", iRange, itEventRange->strEvent1.c_str());
        else
          printf("convertEventRangeToNo(): [%d] Cannot find event with the specified time %s\n", iRange, itEventRange->strEvent1.c_str());
        continue;
      }

      printf("Time %s%s is mapped to Calib# %d Event# %d\n", 
        itEventRange->strEvent1.c_str(), (bExactMatch? " [exact]":""), iCalib1, iEvent1);
      
      if (itEventRange->strEvent2.empty())
      {
        lEventNo.push_back(EventNo(iCalib1, iEvent1));
        continue;
      }
            
      int iCalib2 = -1;
      int iEvent2 = -1;
      iError = run.findTime(itEventRange->strEvent2.c_str(), iCalib2, iEvent2, bExactMatch, bOvertime);      
      if (iError != 0)
      {
        if (bOvertime)
          printf("convertEventRangeToNo(): [%d] Time %s is later than the last event\n", iRange, itEventRange->strEvent2.c_str());
        else
          printf("convertEventRangeToNo(): [%d] Cannot find event with the specified time %s\n", iRange, itEventRange->strEvent2.c_str());
        continue;
      }

      printf("Time %s%s is mapped to Calib# %d Event# %d\n", 
        itEventRange->strEvent2.c_str(), (bExactMatch? " [exact]":""), iCalib2, iEvent2);
      
      for (int iCalib = iCalib1; iCalib <= iCalib2; ++iCalib)
      {          
        int iStartEvent = (iCalib == iCalib1 ? iEvent1 : 1);
        
        int iNumEvent = -1;
        int iError = run.numEventInCalib(iCalib, iNumEvent);
        if (iError != 0)
        {
          printf("convertEventRangeToNo(): [%d] Quert Event# in Calib# %d failed\n", iRange, iCalib);
          continue;
        }
        
        int iEndEvent = (iCalib == iCalib2 ? iEvent2 : iNumEvent);
        for (int iEventNo = iStartEvent; iEventNo <= iEndEvent; ++iEventNo)
          lEventNo.push_back(EventNo(iCalib, iEventNo));                
      } //for (int iCalib = iCalib1; iCalib < iCalib2; ++iCalib)
      
    } // else (itEventRange->event1 > 0)
  } // for (TEventRangeList::const_iterator...
  
  //{//!!debug
  //  printf("Dumping Event list...\n");
  //  int iEvent = 0;
  //  for (TEventNoList::iterator 
  //    it = lEventNo.begin();
  //    it != lEventNo.end(); 
  //    it++, iEvent++ )
  //  {
  //    printf("[%d] C%d %d\n", iEvent, it->calib, it->event );
  //  }   
  //}
    
  return 0;
}

XtcRun* getDarkFrameRun(unsigned run_number)
{
  if (calib_files.empty())
    return 0;

  XtcRun& run = *new XtcRun;
  std::list<std::string>::const_iterator it=calib_files.begin();
  run.reset(*it);
  int nfiles=1;
  while(++it!=calib_files.end()) {
    if (!run.add_file(*it)) {
      XtcRun next;
      next.reset(*it);
      if (next.run_number() > run_number)
        break;
      run.reset(*it);
      nfiles=0;
    }
    nfiles++;
  }

  run.init();

  return &run;
}

void signalHandler( int iSignalNo )
{
  printf( "\nsignalHandler(): signal %d received.\n", iSignalNo );    
}

int main(int argc, char *argv[])
{
  int c;
  char *    xtcname         = 0;
  char *    filelist        = 0;
  char *    runPrefix       = 0;
  char *    caliblist       = 0;
  char *    evtLstFn        = 0;
  char*     reorder_file    = 0;
  int       parseErr        = 0;
  unsigned  skip            = 0;
  unsigned  maxevt          = 0xffffffff;
  unsigned  jump            = 0;
  unsigned  calib           = 0;  
  int       iDebugLevel     = 0;
  unsigned  split_depth     = 0;
  char*     sTime           = 0;
  uint32_t  uFiducialSearch = (uint32_t) -1;
  int       iFidFromEvent   = 1;
  _writer = NULL;

  while ((c = getopt(argc, argv, "hf:l:r:n:d:c:s:O:o:LSy:j:t:u:e:")) != -1)
    {
      switch (c)
        {
        case 'h':
          usage(argv[0]);
          exit(0);
        case 'f':
          xtcname   = optarg;
          break;
        case 'l':
          filelist  = optarg;
          break;
        case 'r':
          runPrefix = optarg;
          break;
        case 'c':
          caliblist = optarg;
          break;
        case 'e':
          evtLstFn  = optarg;
          break;
        case 'n':
          maxevt = strtoul(optarg, NULL, 0);
          printf("Will process %d events\n", maxevt);
          break;
        case 's':
          skip = strtoul(optarg, NULL, 0);
          printf("Will skip first %d events\n", skip);
          break;
        case 'j':
          jump = strtoul(optarg, NULL, 0);
          printf("Will jump to Event# %d (After Calib Cycle)\n", jump);
          break;
        case 'y':
          calib = strtoul(optarg, NULL, 0);
          printf("Will jump to Calib# %d\n", calib);
          break;
        case 't':
          sTime = optarg;
          break;
        case 'u':
          uFiducialSearch = strtoul(optarg, NULL, 0);
          {
          char* sNextParam = strchr(optarg,',');
          if (sNextParam != NULL)
            iFidFromEvent = strtoul(sNextParam+1, NULL, 0);
          }
          break;
        case 'd':
          iDebugLevel = strtoul(optarg, NULL, 0);
          break;
        case 'L':
          XtcRun::live_read(true);
          break;
        case 'o':
          _writer = new XtcWriter(optarg);
          break;
        case 'O':
          reorder_file = optarg;
          break;
        case 'S':
          split_depth = 64;
          break;
        default:
          parseErr++;
        }
    }
      
  /*
   * Register singal handler
   */
  //struct sigaction sigActionSettings;
  //sigemptyset(&sigActionSettings.sa_mask);
  //sigActionSettings.sa_handler = signalHandler;
  //sigActionSettings.sa_flags   = SA_RESTART;    

  //for (int iSignalNo=0; iSignalNo < 64; ++iSignalNo)
  //  sigaction(iSignalNo, &sigActionSettings, 0);
    
  if ((!xtcname && !filelist && !runPrefix) || parseErr)
  {
    if (evtLstFn != 0)
    {
      TEventRangeList lEventRange;
      readEventList(evtLstFn, lEventRange);
      int iEvent = 0;
      for (TEventRangeList::iterator 
        it = lEventRange.begin();
        it != lEventRange.end(); 
        it++, iEvent++ )
      {
        if (it->event1 > 0)
        {
          if (it->event2 < 0)
            printf("[%d] C%d %d\n", iEvent, it->calib, it->event1 );
          else
            printf("[%d] C%d %d-%d\n", iEvent, it->calib, it->event1, it->event2);
        }
        else
          if (it->strEvent2.empty())
            printf("[%d] %s\n", iEvent, it->strEvent1.c_str());
          else
            printf("[%d] \"%s\"-\"%s\"\n", iEvent, it->strEvent1.c_str(), it->strEvent2.c_str());        
      }               
    }
          
      usage(argv[0]);
      exit(2);
  }

  char outfile[200];
  char filename[200];  
  if (xtcname)
    makeoutfilename(xtcname, outfile);
  else if (filelist)
    makeoutfilename(filelist, outfile);
  else if (runPrefix)
    makeoutfilename(runPrefix, outfile);

//  printf("Opening ROOT output file %s\n", outfile);
//  TFile *out;
//  out = new TFile(outfile, "RECREATE");

  _split = new SplitEventQ(split_depth);

  if (caliblist) {
    printf("Opening caliblist %s\n", caliblist);
    FILE *flist = fopen(caliblist, "r");
    if (flist)
      {
        //
        //  create a sorted list of filenames
        //    (this conveniently groups chunks and slices)
        //      
        while (fscanf(flist, "%s", filename) != EOF)
          calib_files.push_back(std::string(filename));
        calib_files.sort();
      }
  }

  TEventRangeList lEventRange;
  if (evtLstFn != 0)
  {
    readEventList(evtLstFn, lEventRange);
    printf( "Read %d event numbers from event list file %s\n", (int) lEventRange.size(), evtLstFn );
  }

  if (xtcname) {
    XtcRun* run_ptr = new XtcRun;
    XtcRun& run = *run_ptr;
    run.reset(std::string(xtcname));
    anarun(run, maxevt, skip, reorder_file, jump, calib, lEventRange, sTime, uFiducialSearch, iFidFromEvent, iDebugLevel);
    //    delete run_ptr;
  }
  
  if (filelist)
  {
    printf("Opening filelist %s\n", filelist);
    FILE *flist = fopen(filelist, "r");
    if (flist)
    {
      currlistname = (char*)malloc(FNAME_LEN);
      strncpy(currlistname,basename(filelist),FNAME_LEN);
      //
      //  create a sorted list of filenames
      //    (this conveniently groups chunks and slices)
      //
      std::list<std::string> all_files;
      while (fscanf(flist, "%s", filename) != EOF)
      {
        printf("main(): Adding file %s to the processing list...\n", filename);
        all_files.push_back(filename);
      }
      all_files.sort();

      XtcRun* run_ptr = new XtcRun;
      XtcRun& run = *run_ptr;
      std::list<std::string>::const_iterator it=all_files.begin();
      run.reset(*it);
      int nfiles=1;
      while(++it!=all_files.end()) {
        if (!run.add_file(*it)) {
          printf("Analyzing files %s [%d] \n",
                 run.base(),nfiles);
	  _xtcfilename = *(--it);
	  ++it;
          anarun(run, maxevt, skip, reorder_file, jump, calib, lEventRange, sTime, uFiducialSearch, iFidFromEvent, iDebugLevel);
          run.reset(*it);
          nfiles=0;
        }
        nfiles++;
      }
      printf("Analyzing files %s [%d]\n",
             run.base(),nfiles);
      _xtcfilename = *(--it);
      ++it;
      anarun(run, maxevt, skip, reorder_file, jump, calib, lEventRange, sTime, uFiducialSearch, iFidFromEvent, iDebugLevel);
      //      delete run_ptr;
    }
    else
      printf("Unable to open list of files %s\n", filelist);
  }

  if (runPrefix)
  {
    string strFnPrefix(runPrefix);  
    size_t uPos = strFnPrefix.find("-r");
    if (uPos == string::npos)
      printf("Invalid run filename %s\n", runPrefix);
    else
    {
      // Get the prefix for <path>/eXX-rXXXX
      string  strFnBase = strFnPrefix.substr(0, uPos+6);      
      std::list<std::string> all_files;
      /*
       * Find out all xtc files within this run
       */       
      for (int iSliceSerial = 0;;++iSliceSerial)
      {
        int iChunkSerial = 0;
        while (true)
        {
          char sFnBuf[128];
          sprintf(sFnBuf, "%s-s%02d-c%02d.xtc", strFnBase.c_str(), iSliceSerial, iChunkSerial);
          
          struct ::stat64 statFile;
          int iError = ::stat64(sFnBuf, &statFile);
          if ( iError != 0 )
            break;            
            
          printf("main(): Adding file %s to the processing list...\n", sFnBuf);
            
          all_files.push_back(sFnBuf);
          ++iChunkSerial;
        }
        
        if (iChunkSerial == 0)
          break;
      } // for (int iSliceSerial = 0;;++iSliceSerial)
        
      all_files.sort();

      XtcRun* run_ptr = new XtcRun;
      XtcRun& run = *run_ptr;
      std::list<std::string>::const_iterator it=all_files.begin();
      run.reset(*it);
      int nfiles=1;
      while(++it!=all_files.end()) {
        if (!run.add_file(*it)) {
          printf("Analyzing files %s [%d]\n", 
                 run.base(),nfiles);
          anarun(run, maxevt, skip, reorder_file, jump, calib, lEventRange, sTime, uFiducialSearch, iFidFromEvent, iDebugLevel);
          run.reset(*it);
          nfiles=0;
        }
        nfiles++;
      }
      printf("Analyzing files %s [%d]\n", 
             run.base(),nfiles);
      anarun(run, maxevt, skip, reorder_file, jump, calib, lEventRange, sTime, uFiducialSearch, iFidFromEvent, iDebugLevel);           
      //      delete run_ptr;
    } // if (uPos != string::npos)
  }
    
  endjob();

  delete _split;

//  out->Write();
//  out->Close();

  if (_writer)
    delete _writer;

  return 0;
}


XtcWriter::XtcWriter(char* path) : _path(path), _ofile(0)
{
  struct stat buf;

  if (path && (stat(path, &buf) != 0)) {
    if (mkdir(path, 0777)) {
      perror("myana::XtcWriter mkdir");
    }
  }
}
  

XtcWriter::~XtcWriter() {}

void XtcWriter::insert(Dgram& dg) 
{
  switch(dg.seq.service()) {
  case TransitionId::L1Accept:
    if (!_event_recorded) {
      for(std::list<char*>::iterator it=_tr.begin(); it!=_tr.end(); it++)
        _write(*reinterpret_cast<Dgram*>(*it));
      _event_recorded = true;
    }
    _write(dg);
    break;
  case TransitionId::Configure:
    _queue(dg);
    break;
  case TransitionId::BeginRun:
    _openFile(dg);
    break;
  case TransitionId::BeginCalibCycle:
  case TransitionId::Enable:
    _queue(dg);
    _event_recorded = false;
    break;
  case TransitionId::Disable:
  case TransitionId::EndCalibCycle:
    _flush(dg);
    break;
  case TransitionId::EndRun:
    _write(dg);
    fclose(_ofile);
    break;
  default:
    break;
  }
}

void XtcWriter::_openFile(Dgram& dg)
{
  char buff[32];
  sprintf(buff,"dst-r%04d-s00-c00.xtc",dg.env.value());
  std::string fname(_path);
  fname += buff;

  _ofile = fopen(fname.c_str(),"wx");
  if (!_ofile) {
    printf("Error opening output file %s.\n",fname.c_str());
    exit(1);
  }

  _write(*reinterpret_cast<Dgram*>(_tr.front()));
  _write(dg);
  delete[] _tr.front();
  _tr.clear();
}

void XtcWriter::_write(Dgram& dg)
{
  fwrite(&dg,sizeof(dg)+dg.xtc.sizeofPayload(),1,_ofile); 
}

void XtcWriter::_queue(Dgram& dg)
{
  unsigned sz = sizeof(dg) + dg.xtc.sizeofPayload();
  char* b = new char[sz];
  memcpy(b, &dg, sz);
  _tr.push_back(b);
}

void XtcWriter::_flush(Dgram& dg)
{
  Dgram* p = reinterpret_cast<Dgram*>(_tr.back());
  if (!_tr.empty() &&
      p->seq.service()==dg.seq.service()-1) {
    delete[] _tr.back();
    _tr.pop_back();
  }
  else
    _write(dg);
}

