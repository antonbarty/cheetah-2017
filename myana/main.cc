/* $Id: main.cc,v 1.72 2011/02/04 22:51:43 weaver Exp $ */
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
/*
#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>
#include <TH1.h>
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <list>
#include <iterator>
#include <poll.h>

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
#include "pdsdata/camera/FrameFexConfigV1.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/camera/TwoDGaussianV1.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/epics/EpicsPvData.hh"
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
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/princeton/ConfigV1.hh"
#include "pdsdata/princeton/FrameV1.hh"
#include "pdsdata/princeton/InfoV1.hh"
#include "pdsdata/ipimb/ConfigV1.hh"
#include "pdsdata/ipimb/DataV1.hh"
#include "pdsdata/encoder/ConfigV1.hh"
#include "pdsdata/encoder/DataV1.hh"
#include "pdsdata/encoder/DataV2.hh"
#include "pdsdata/lusi/DiodeFexConfigV1.hh"
#include "pdsdata/lusi/DiodeFexV1.hh"
#include "pdsdata/lusi/IpmFexConfigV1.hh"
#include "pdsdata/lusi/IpmFexV1.hh"
#include "pdsdata/cspad/ConfigV1.hh"
#include "pdsdata/cspad/ConfigV2.hh"
#include "pdsdata/cspad/ConfigV3.hh"
#include "pdsdata/cspad/ElementIterator.hh"

#include "main.hh"
#include "myana.hh"
#include "XtcRun.hh"

using std::vector;
using std::string;
using std::map;
using namespace Pds;

static string GenerateCtrlPvHashKey( const char* pvName, int arrayIndex);

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
  private:
    unsigned _level;
    unsigned _info;
    TypeId   _t;
  };

public:
  EventStore(int iDebugLevel) : _iDebugLevel(iDebugLevel) 
  {
    _reset_control();
  }
  ~EventStore() 
  {
    for(map<Key,const Xtc*>::iterator it=_cmaps.begin(); it!=_cmaps.end(); it++)
      delete[] reinterpret_cast<const char*>(it->second);
  }
public:
  void processDg(Dgram* dg) 
  {
    _seq = dg->seq;

    if (_seq.service()==TransitionId::BeginRun)
      _repics.clear();

    if (_seq.service()==TransitionId::BeginCalibCycle)
      _reset_control();

    if (_seq.service()==TransitionId::L1Accept)
      _rmaps.clear();

    iterate(&dg->xtc);
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
    else if (xtc->damage.value()) { // skip damaged detector data
    }
    else {                          // handle specific detector data
      const DetInfo& info = *(DetInfo*)(&xtc->src);
      if (_iDebugLevel >= 1)
        printf( "> Xtc type %-16s  Detector %-12s:%d  Device %-12s:%d\n", 
                TypeId::name(xtc->contains.id()),
                DetInfo::name(info.detector()), info.detId(), 
                DetInfo::name(info.device()), info.devId() );

      if (xtc->contains.id() == TypeId::Id_ControlConfig)
        _store_control(xtc);
      else if (xtc->contains.id() == TypeId::Id_Epics)
        _store_epics(xtc);
      else
        _store(info, xtc->contains, xtc);
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
    if (_seq.service()==TransitionId::L1Accept)
      _rmaps[key] = xtc;
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
    const EpicsPvHeader& epicsPv = *reinterpret_cast<const EpicsPvHeader*>(xtc->payload());
    if ( dbr_type_is_CTRL(epicsPv.iDbrType) ) {
      if ( epicsPv.iPvId < (int)_repics.size() )
        printf( "myLevelIter::process(): epics control data (id %d) is duplicated\n", epicsPv.iPvId );
      _epics_name_map[ ((const EpicsPvCtrlHeader&)epicsPv).sPvName ] = epicsPv.iPvId;            
    }
    if ( epicsPv.iPvId >= (int) _repics.size() ) 
      _repics.resize( epicsPv.iPvId+1 );
    _repics[ epicsPv.iPvId ] = &epicsPv;
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
private:
  int      _iDebugLevel;
  Sequence _seq;
  // xtc data
  map<Key,const Xtc*> _rmaps;  // reference to data
  map<Key,const Xtc*> _cmaps;  // copy of data
  // epics data
  map<string,int>              _epics_name_map;
  vector<const EpicsPvHeader*> _repics;  // reference to epics data
  // calib data
  vector<unsigned char> _control_buffer;
  map<string, int>      _control_name_map;
  map<string, int>      _monitor_name_map;
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
/*
void fillConstFrac(double* t, double* v, unsigned numSamples, float baseline,
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
static unsigned char  imagePnCcd[iPnCcdImageSize];

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
                                  OPAL_INFO(XppEndstation,0) };
  return info[det];
}

/*
 * Configure data retrieval functions
 */
#define TYPEID(t,v) TypeId(TypeId::Id_##t,v)

int getAcqConfig(DetInfo det, int& numChannels, int& numSamples, double& sampleInterval)
{
  Xtc* xtc = const_cast<Xtc*>(_estore->lookup_cfg( det, TYPEID(AcqConfig,1) ));
  if (xtc) {
    Acqiris::ConfigV1& acqCfg = *reinterpret_cast<Acqiris::ConfigV1*>(xtc->payload());
    const Acqiris::HorizV1& horiz = acqCfg.horiz();
    
    numChannels = acqCfg.nbrChannels();
    numSamples = horiz.nbrSamples();
    sampleInterval = horiz.nbrSamples() * horiz.sampInterval();    
    return 0;
  }
  else
    return 2; // Data has never been processed or read from xtc
}

int getAcqConfig(AcqDetector det, int& numChannels, int& numSamples, double& sampleInterval)
{
  return getAcqConfig( AcqDetectorIndex(det), numChannels, numSamples, sampleInterval );
}

 
int getFrameConfig(DetInfo info)
{
  return (_estore->lookup_cfg( info, TYPEID(FrameFexConfig,1) )==0) ? 2 : 0;
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
  if (xtc) {
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
    return 0;
  }
  else
    return 2;
}

int getDiodeFexConfig (DetInfo::Detector det, int iDevId, float* base, float* scale)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Ipimb,0), TypeId(TypeId::Id_DiodeFexConfig,1) );
  if (xtc) {
    const Lusi::DiodeFexConfigV1* p = reinterpret_cast<const Lusi::DiodeFexConfigV1*>(xtc->payload());
    for(unsigned i=0; i<Lusi::DiodeFexConfigV1::NRANGES; i++) {
      base [i] = p->base [i];
      scale[i] = p->scale[i];
    }
    return 0;
  }
  else
    return 2;
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
  if (xtc) {
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
    return 0;
  }
  else
    return 2;
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


int getEpicsPvNumber()
{
  return _estore->epics().size();
}

int getEpicsPvConfig( int pvId, const char*& pvName, int& type, int& numElements )
{
  if ( pvId < 0 || pvId >= (int) _estore->epics().size() ) 
    { printf( "getEpicsPvConfig(): PV Id (%d) is invalid.\n", pvId ); return 1; }  
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
  if (xtc) {
    const Princeton::ConfigV1& princetonCfg = *reinterpret_cast<const Princeton::ConfigV1*>(xtc->payload());
    width   = princetonCfg.width  ();
    height  = princetonCfg.height ();
    orgX    = princetonCfg.orgX   ();
    orgY    = princetonCfg.orgY   ();
    binX    = princetonCfg.binX   ();
    binY    = princetonCfg.binY   ();
    return 0;
  }
  else
    return 2;
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
  if (xtc) {
    const Ipimb::ConfigV1& cfg = *reinterpret_cast<const Ipimb::ConfigV1*>(xtc->payload());
    serialID = cfg.serialID();
    chargeAmpRange0 = cfg.chargeAmpRange() & 0x3;
    chargeAmpRange1 = (cfg.chargeAmpRange() >> 2) & 0x3;
    chargeAmpRange2 = (cfg.chargeAmpRange() >> 4) & 0x3;
    chargeAmpRange3 = (cfg.chargeAmpRange() >> 6) & 0x3;
    return 0;
  }
  else
    return 2;
}

int getEncoderConfig(DetInfo::Detector det, int iDevId)
{
  const Xtc* xtc = _estore->lookup_cfg( DetInfo(0,det,0,DetInfo::Encoder,0),
                                        TypeId (TypeId::Id_EncoderConfig,2) );
  if (xtc)
    return 0;
  else
    return 2;
}

/*
 * L1Accept data retrieval functions
 */
int getEvrDataNumber()
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,DetInfo::NoDetector,0,
                                                DetInfo::Evr,0),
                                        TypeId (TypeId::Id_EvrData,3) );
  if (xtc)
    return reinterpret_cast<const EvrData::DataV3*>(xtc->payload())->numFifoEvents();
  else
    return 0;
}

int getEvrData( int id, unsigned int& eventCode, unsigned int& fiducial, unsigned int& timeStamp )
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,DetInfo::NoDetector,0,
                                                DetInfo::Evr,0),
                                        TypeId (TypeId::Id_EvrData,3) );
  if (xtc) {
    const EvrData::DataV3::FIFOEvent& fifoEvent = 
      reinterpret_cast<const EvrData::DataV3*>(xtc->payload())->fifoEvent(id);
    eventCode = fifoEvent.EventCode;
    fiducial  = fifoEvent.TimestampHigh;
    timeStamp = fifoEvent.TimestampLow;
    return 0;
  }
  else
    return 2;
}

int getAcqValue(DetInfo info, int channel, double*& time, double*& voltage, double& trigtime)
{
  const Xtc* xtc = _estore->lookup_evt( info, TypeId(TypeId::Id_AcqWaveform,1) );
  if (xtc) {

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
    return 0;
  }
  else
    return 2;
}

int getAcqValue(AcqDetector det, int channel, double*& time, double*& voltage, double& trigtime)
{ return getAcqValue( AcqDetectorIndex( det ), channel, time, voltage, trigtime ); }

int getAcqValue(AcqDetector det, int channel, double*& time, double*& voltage)
{ double trigtime; return getAcqValue(det, channel, time, voltage, trigtime); }

int getFrameValue(FrameDetector det, int& frameWidth, int& frameHeight, unsigned short*& image )
{
  return getFrameValue( FrameDetectorIndex(det), frameWidth, frameHeight, image);
}

int getFrameValue(DetInfo info, int& frameWidth, int& frameHeight, unsigned short*& image )
{
  const Xtc* xtc = _estore->lookup_evt( info, TypeId(TypeId::Id_Frame,1) );
  if (xtc) {
    const Camera::FrameV1& frame = *reinterpret_cast<const Camera::FrameV1*>(xtc->payload());
    frameWidth  = frame.width();
    frameHeight = frame.height();
    image       = (unsigned short*) frame.data();
    return 0;
  }
  else
    return 2;
}

int getPrincetonValue(DetInfo::Detector det, int iDevId, unsigned short *& image)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Princeton,iDevId),
                                        TypeId(TypeId::Id_PrincetonFrame,1) );
  if (xtc) {
    const Princeton::FrameV1& princetonFrame = *reinterpret_cast<const Princeton::FrameV1*>(xtc->payload());
    image = (unsigned short*) princetonFrame.data();
    return 0;
  }
  return 2;
}

int getPrincetonTemperature(DetInfo::Detector det, int iDevId, float & fTemperature)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Princeton,iDevId),
                                        TypeId(TypeId::Id_PrincetonInfo,1) );
  if (xtc) {
    const Princeton::InfoV1& princetonInfo = *reinterpret_cast<Princeton::InfoV1*>(xtc->payload());
    fTemperature = princetonInfo.temperature();
    return 0;
  }
  else
    return 2;
}

int getIpimbVolts(DetInfo::Detector det, int iDevId,
                  float &channel0, float &channel1, float &channel2, float &channel3)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Ipimb,iDevId),
                                        TypeId(TypeId::Id_IpimbData,1) );
  if (xtc) {
    const Ipimb::DataV1& ipimbData = *reinterpret_cast<const Ipimb::DataV1*>(xtc->payload());
    channel0 = ipimbData.channel0Volts();
    channel1 = ipimbData.channel1Volts();
    channel2 = ipimbData.channel2Volts();
    channel3 = ipimbData.channel3Volts();
    return 0;
  }
  else
    return 2;
}

int getBldIpimbVolts(BldInfo::Type bldType, float &channel0, float &channel1,
                     float &channel2, float &channel3)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,bldType),
                                        TypeId(TypeId::Id_IpimbData,1) );
  if (xtc) {
    const Ipimb::DataV1& bldIpimbData = *reinterpret_cast<const Ipimb::DataV1*>(xtc->payload());
    channel0 = bldIpimbData.channel0Volts();
    channel1 = bldIpimbData.channel1Volts();
    channel2 = bldIpimbData.channel2Volts();
    channel3 = bldIpimbData.channel3Volts();
    return 0;
  }
  else
    return 2;
}

int getBldIpimbConfig(BldInfo::Type bldType, uint64_t& serialID,
                      int& chargeAmpRange0, int& chargeAmpRange1,
                      int& chargeAmpRange2, int& chargeAmpRange3)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,bldType),
                                        TypeId(TypeId::Id_IpimbConfig,1) );
  if (xtc) {
    const Ipimb::ConfigV1& bldIpimbConfig = *reinterpret_cast<const Ipimb::ConfigV1*>(xtc->payload());
    serialID = bldIpimbConfig.serialID();
    chargeAmpRange0 = bldIpimbConfig.chargeAmpRange() & 0x3;
    chargeAmpRange1 = (bldIpimbConfig.chargeAmpRange() >> 2) & 0x3;
    chargeAmpRange2 = (bldIpimbConfig.chargeAmpRange() >> 4) & 0x3;
    chargeAmpRange3 = (bldIpimbConfig.chargeAmpRange() >> 6) & 0x3;
    return 0;
  }
  else
    return 2;
}

int getBldIpmFexValue   (BldInfo::Type bldType, float* channels, 
                         float& sum, float& xpos, float& ypos)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,bldType),
                                        TypeId(TypeId::Id_IpimbConfig,1) );
  if (xtc) {
    const Lusi::IpmFexV1& bldIpmFexData = *reinterpret_cast<const Lusi::IpmFexV1*>(xtc->payload());
    for(unsigned i=0; i<Lusi::IpmFexConfigV1::NCHANNELS; i++) 
      channels[i] = bldIpmFexData.channel[i];
    sum  = bldIpmFexData.sum;
    xpos = bldIpmFexData.xpos;
    ypos = bldIpmFexData.ypos;
    return 0;
  }
  else
    return 2;
}


int getEncoderCount(DetInfo::Detector det, int iDevId, int& encoderCount, int chan)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Encoder,iDevId),
                                        TypeId(TypeId::Id_EncoderData,2) );
  if (xtc) {
    encoderCount = reinterpret_cast<const Encoder::DataV2*>(xtc->payload())->value(chan);
  }
  else {
    const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Encoder,iDevId),
                                          TypeId(TypeId::Id_EncoderData,1) );
    if (xtc) {
      encoderCount = reinterpret_cast<const Encoder::DataV1*>(xtc->payload())->value();
    }
    else
      return 2;
  }
  return 0;
}

int getDiodeFexValue (DetInfo::Detector det, int iDevId, float& value)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Ipimb,iDevId),
                                        TypeId(TypeId::Id_DiodeFex,1) );
  if (xtc) {
    value = reinterpret_cast<const Lusi::DiodeFexV1*>(xtc->payload())->value;
    return 0;
  }
  else
    return 2;
}

int getIpmFexValue   (DetInfo::Detector det, int iDevId, 
                      float* channels, float& sum, float& xpos, float& ypos)
{
  const Xtc* xtc = _estore->lookup_evt( DetInfo(0,det,0,DetInfo::Ipimb,iDevId),
                                        TypeId(TypeId::Id_IpmFex,1) );
  if (xtc) {
    const Lusi::IpmFexV1* p = reinterpret_cast<const Lusi::IpmFexV1*>(xtc->payload());
    for(unsigned i=0; i<Lusi::IpmFexConfigV1::NCHANNELS; i++) 
      channels[i] = p->channel[i];
    sum  = p->sum;
    xpos = p->xpos;
    ypos = p->ypos;
    return 0;
  }
  else
    return 2;
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
	
	return 2;
}


int getFeeGasDet(double* shotEnergy)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,BldInfo::FEEGasDetEnergy),
                                        TypeId (TypeId::Id_FEEGasDetEnergy,1) );
  if (xtc) {
    const BldDataFEEGasDetEnergy* pBldFeeGasDetEnergy = 
      reinterpret_cast<const BldDataFEEGasDetEnergy*>(xtc->payload());
    shotEnergy[0] = pBldFeeGasDetEnergy->f_11_ENRC;
    shotEnergy[1] = pBldFeeGasDetEnergy->f_12_ENRC;
    shotEnergy[2] = pBldFeeGasDetEnergy->f_21_ENRC;
    shotEnergy[3] = pBldFeeGasDetEnergy->f_22_ENRC;
    return 0;
  }
  else
    return 2;
}

int getPhaseCavity(double& fitTime1, double& fitTime2,
                   double& charge1,  double& charge2)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,BldInfo::PhaseCavity),
                                        TypeId (TypeId::Id_PhaseCavity,1) );
  if (xtc) {
    const BldDataPhaseCavity* pBldPhaseCavity = 
      reinterpret_cast<const BldDataPhaseCavity*>(xtc->payload());
    fitTime1 = pBldPhaseCavity->fFitTime1;
    fitTime2 = pBldPhaseCavity->fFitTime2;
    charge1  = pBldPhaseCavity->fCharge1;
    charge2  = pBldPhaseCavity->fCharge2;
    return 0;
  }
  else
    return 2;
}

int getEBeam(double& charge, double& energy, double& posx, double& posy,
             double& angx, double& angy, double& pkcurr)
{
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,BldInfo::EBeam),
                                        TypeId (TypeId::Id_EBeam,1) );
  if (xtc) {
    const BldDataEBeam* pBldEBeam = 
      reinterpret_cast<const BldDataEBeam*>(xtc->payload());
    charge = pBldEBeam->fEbeamCharge;    /* in nC */ 
    energy = pBldEBeam->fEbeamL3Energy;  /* in MeV */ 
    posx   = pBldEBeam->fEbeamLTUPosX;   /* in mm */ 
    posy   = pBldEBeam->fEbeamLTUPosY;   /* in mm */ 
    angx   = pBldEBeam->fEbeamLTUAngX;   /* in mrad */ 
    angy   = pBldEBeam->fEbeamLTUAngY;   /* in mrad */  
    pkcurr = pBldEBeam->fEbeamPkCurrBC2; /* in Amps */
    return 0;
  }
  else
    return 2;
}

int getEBeam(double& charge, double& energy, double& posx, double& posy,
             double& angx, double& angy)
{
  double pkc; 
  if ( getEBeam(charge, energy, posx, posy, angx, angy, pkc) == 0 )
    return 0;
    
  const Xtc* xtc = _estore->lookup_evt( BldInfo(0,BldInfo::EBeam),
                                        TypeId (TypeId::Id_EBeam,0) );
  if (xtc) {
    const BldDataEBeamV0* pBldEBeamV0 = 
      reinterpret_cast<const BldDataEBeamV0*>(xtc->payload());
    charge = pBldEBeamV0->fEbeamCharge;    /* in nC */ 
    energy = pBldEBeamV0->fEbeamL3Energy;  /* in MeV */ 
    posx   = pBldEBeamV0->fEbeamLTUPosX;   /* in mm */ 
    posy   = pBldEBeamV0->fEbeamLTUPosY;   /* in mm */ 
    angx   = pBldEBeamV0->fEbeamLTUAngX;   /* in mrad */ 
    angy   = pBldEBeamV0->fEbeamLTUAngY;   /* in mrad */  
    return 0;
  }
  else
    return 2;
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
      //printf( "getPvInt(%s): Not an floating point value PV.\n", pvName );    
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

static const DetInfo::Detector pnccdDets[] = { DetInfo::Camp, DetInfo::SxrEndstation };
static const unsigned npnccdDets =  sizeof(pnccdDets)/sizeof(DetInfo::Detector);

int getPnCcdConfig  ( int deviceId, const PNCCD::ConfigV1*& c )
{
  const Xtc* xtc = 0;
  for(unsigned idet=0; idet<npnccdDets; idet++)
    if ((xtc = _estore->lookup_cfg( DetInfo(0,pnccdDets[idet],0,DetInfo::pnCCD,deviceId),
                                    TypeId (TypeId::Id_IpimbConfig,1) )))
      break;
        
  if (xtc) {
    c = reinterpret_cast<const PNCCD::ConfigV1*>(xtc->payload());
    return 0;
  }
  else
    return 2;
}

int getPnCcdRaw  ( int deviceId, const PNCCD::FrameV1*& frame )
{
  const Xtc* xtc = 0;
  for(unsigned idet=0; idet<npnccdDets; idet++)
    if ((xtc = _estore->lookup_evt( DetInfo(0,pnccdDets[idet],0,DetInfo::pnCCD,deviceId),
                                    TypeId (TypeId::Id_IpimbConfig,1) )))
      break;
        
  if (xtc) {
    frame = reinterpret_cast<const PNCCD::FrameV1*>(xtc->payload());
    return 0;
  }
  else
    return 2;
}

int getPnCcdValue( int deviceId, unsigned char*& image, int& width, int& height )
{
  const Xtc* xtc = 0;
  unsigned idet=0;
  while(idet<npnccdDets) {
    if ((xtc = _estore->lookup_evt( DetInfo(0,pnccdDets[idet],0,DetInfo::pnCCD,deviceId),
                                    TypeId (TypeId::Id_IpimbConfig,1) )))
      break;
    idet++;
  }
        
  if (xtc) {

    const PNCCD::ConfigV1&  config  = *reinterpret_cast<const PNCCD::ConfigV1*>
      ( _estore->lookup_cfg( DetInfo(0,pnccdDets[idet],0,DetInfo::pnCCD,deviceId),
                             TypeId (TypeId::Id_pnCCDframe,1) )->payload() );
    const PNCCD::FrameV1*   pFrame0 =  reinterpret_cast<const PNCCD::FrameV1*>(xtc->payload());
    const PNCCD::FrameV1*   pFrame1 = pFrame0->next(config);
    const PNCCD::FrameV1*   pFrame2 = pFrame1->next(config);
    const PNCCD::FrameV1*   pFrame3 = pFrame2->next(config);  
  
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
    return 0;
  }
  else
    return 2;
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
  fprintf(stderr,"Usage: %s -f <filename> | -l <filewithfilelist> [-c <caliblist>] [-s <skipevts>] [-n <maxevts>] [-d <debug level>] [-h]\n", progname);
  fprintf(stderr,"Usage: The -l and -c arguments require files with a list of files in them.\n");
}

void makeoutfilename(char* filename, char* outfilename) 
{
  unsigned start= 0;
  unsigned last = strlen(filename);
  unsigned end    = last;
  for (unsigned i=0;i<last;i++) 
    {
      if (filename[i]=='/') start=i+1;
      if (filename[i]=='.') end=i-1;
    }
  strncpy(outfilename,filename+start,end-start+1);
  strncpy(outfilename+end-start+1,".root",6);
}

static void dump(Dgram* dg)
{
  char buff[128];
  time_t t = dg->seq.clock().seconds();
  strftime(buff,128,"%H:%M:%S",localtime(&t));
  printf("%s.%09u %08x/%08x %s extent 0x%x damage %x\n",
         buff,
         dg->seq.clock().nanoseconds(),
         dg->seq.stamp().fiducials(),dg->seq.stamp().vector(),
         TransitionId::name(dg->seq.service()),
         dg->xtc.extent, dg->xtc.damage.value());
}

static void dump(Dgram* dg, unsigned ev)
{
  char buff[128];
  time_t t = dg->seq.clock().seconds();
  strftime(buff,128,"%H:%M:%S",localtime(&t));
  printf("%s.%09u %08x/%08x %s extent 0x%x damage %x event %d\n",
         buff,
         dg->seq.clock().nanoseconds(),
         dg->seq.stamp().fiducials(),dg->seq.stamp().vector(),
         TransitionId::name(dg->seq.service()),
         dg->xtc.extent, dg->xtc.damage.value(), ev);
}

void anarun(XtcRun& run, unsigned &maxevt, unsigned &skip, int iDebugLevel)
{
  run.init();

  char* buffer = new char[0x2000000];
  Dgram* dg = (Dgram*)buffer;
  printf("Using buffer %p\n",buffer);

  _estore = new EventStore(iDebugLevel);

  Result r = OK;
  unsigned nevent = 0;
  unsigned nprint = 1;
  unsigned damage;
  unsigned damagemask = 0;
  unsigned ndamage = 0;
  do {
    
    r = run.next(dg);
    if (r == Error)
      break;

    if (dg->seq.service()!=TransitionId::L1Accept) 
      dump(dg);
    else if (skip) {
      skip--;
      continue;
    }
    else if (nevent%nprint == 0) {
      dump(dg,nevent+1);
      if (nevent==10*nprint)
        nprint *= 10;
    }

    _estore->processDg(dg);

    damage = dg->xtc.damage.value();
    if (damage)
      {
        ndamage++;
        damagemask |= damage;
      }

    if (dg->seq.service() == TransitionId::L1Accept)
      {
        if (maxevt)
          maxevt--;
        else
          break;
        nevent++;

        event();
      }
    else {
      if (dg->seq.service() == TransitionId::Configure)
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
          
          beginrun();
        }    
      else if (dg->seq.service() == TransitionId::BeginCalibCycle)
        {
          begincalib();
        }
      else if (dg->seq.service() == TransitionId::EndCalibCycle)
        {
          endcalib();
        }
    }
  } while(r==OK);
  
  endrun();
  printf("Processed %d events, %d damaged, with damage mask 0x%x.\n", nevent,
         ndamage, damagemask);

  delete[] buffer;
  delete _estore;
}

std::list<std::string> calib_files;

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


int main(int argc, char *argv[])
{
  int c;
  char *xtcname = 0;
  char *filelist  = 0;
  char *caliblist = 0;
  char  filename[200];
  int parseErr = 0;
  unsigned skip = 0;
  unsigned maxevt = 0xffffffff;
  int iDebugLevel = 0;

  while ((c = getopt(argc, argv, "hf:n:d:l:c:s:L")) != -1)
    {
      switch (c)
        {
        case 'h':
          usage(argv[0]);
          exit(0);
        case 'f':
          xtcname = optarg;
          break;
        case 'l':
          filelist = optarg;
          break;
        case 'c':
          caliblist = optarg;
          break;
        case 'n':
          maxevt = strtoul(optarg, NULL, 0);
          printf("Will process %d events.\n", maxevt);
          break;
        case 's':
          skip = strtoul(optarg, NULL, 0);
          printf("Will skip first %d events.\n", skip);
          break;
        case 'd':
          iDebugLevel = strtoul(optarg, NULL, 0);
          break;
        case 'L':
          XtcSlice::live_read(true);
          break;
        default:
          parseErr++;
        }
    }

  if ((!xtcname && !filelist) || parseErr)
    {
      usage(argv[0]);
      exit(2);
    }

  char outfile[200];
  if (xtcname)
    makeoutfilename(xtcname, outfile);
  if (filelist)
    makeoutfilename(filelist, outfile);
  
/*
  printf("Opening output file %s\n", outfile);
  TFile *out;
  out = new TFile(outfile, "RECREATE");
*/
  
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

  if (xtcname) {
    XtcRun run;
    run.reset(std::string(xtcname));
    anarun(run, maxevt, skip, iDebugLevel);
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
            all_files.push_back(std::string(filename));
          all_files.sort();

          XtcRun run;
          std::list<std::string>::const_iterator it=all_files.begin();
          run.reset(*it);
          int nfiles=1;
          while(++it!=all_files.end()) {
            if (!run.add_file(*it)) {
              printf("Analyzing files %s [%d]\n", 
                     run.base(),nfiles);
              anarun(run, maxevt, skip, iDebugLevel);
              run.reset(*it);
              nfiles=0;
            }
            nfiles++;
          }
          printf("Analyzing files %s [%d]\n", 
                 run.base(),nfiles);
          anarun(run, maxevt, skip, iDebugLevel);
        }
      else
        printf("Unable to open list of files %s\n", filelist);
    }
  endjob();
/*
  out->Write();
  out->Close();
*/
  return 0;
}


