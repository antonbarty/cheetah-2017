/* $Id: main.cc,v 1.66 2010/11/02 16:52:54 weaver Exp $ */
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

//#include <TROOT.h>
//#include <TApplication.h>
//#include <TFile.h>
//#include <TH1.h>

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
#include "pdsdata/cspad/ElementIterator.hh"

#include "main.hh"
#include "myana.hh"
#include "XtcRun.hh"

using std::vector;
using std::string;
using std::map;
using std::auto_ptr;

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

//void fillConstFrac(double* t, double* v, unsigned numSamples, float baseline,
//                   float thresh, TH1* hist) {
//  double edge[100];
//  int n;
//  fillConstFrac(t,v,numSamples,baseline,thresh,edge,n,100);
//  for(int i=0; i<n; i++)
//    hist->Fill(edge[i],1.0);
//}

/* 
 * Time Data
 */
Pds::ClockTime clockTimeCurDatagram;
unsigned _fiducials;

int getTime( int& seconds, int& nanoSeconds )
{
  seconds = clockTimeCurDatagram.seconds();
  nanoSeconds = clockTimeCurDatagram.nanoseconds();
  return 0;
}
 
int getLocalTime( const char*& time )
{
  static const char timeFormatStr[40] = "%04Y-%02m-%02d %02H:%02M:%02S"; /* Time format string */    
  static char sTimeText[40];
    
  int seconds = clockTimeCurDatagram.seconds();
  struct tm tmTimeStamp;
  localtime_r( (const time_t*) (void*) &seconds, &tmTimeStamp );    
  strftime(sTimeText, sizeof(sTimeText), timeFormatStr, &tmTimeStamp );
    
  time = sTimeText;
  return 0;
}

int getFiducials(unsigned& fiducials)
{
  fiducials = _fiducials;
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
 * Check-in log. Used to record which detector/epics pv has been read from the current datagram
 */ 
static unsigned char            lcAcqProcessed      [NumAcqDetector];
static unsigned char            lcFrameProcessed    [NumFrameDetector];
static vector<unsigned char>    vcEpicsProcessed;
static unsigned char            lcPnCcdProcessed    [NumPnCcdDetector];
static bool                     bControlProcessed;
static unsigned char            lcPrincetonProcessed[NumPrincetonDetector];
static unsigned char            lcIpimbProcessed    [NumIpimbDetector];
static unsigned char            lcEncoderProcessed  [NumEncoderDetector];

/*
 * Configuration data
 */
static Acqiris::ConfigV1                lAcqConfig      [NumAcqDetector];
static const Opal1k::ConfigV1*          lpOpal1kConfig  [NumFrameDetector];
static const Pulnix::TM6740ConfigV1*    lpTm6740ConfigV1[NumFrameDetector];
static const Pulnix::TM6740ConfigV2*    lpTm6740Config  [NumFrameDetector];
static const Camera::FrameFexConfigV1*  lpFrameFexConfig[NumFrameDetector];
static vector<const EpicsPvHeader*>     vpEpicsConfig;
static map<string, int>                 mapEpicsPvNameToIndex;
static PNCCD::ConfigV1                  lpnCcdCfg       [NumPnCcdDetector];
static Princeton::ConfigV1              lprincetonCfg   [NumPrincetonDetector];
static Ipimb::ConfigV1                  lipimbCfg       [NumIpimbDetector];
static Encoder::ConfigV1                lencoderCfg     [NumEncoderDetector];
static FCCD::FccdConfigV2               lFccdConfig     [NumFrameDetector];

/*
 * L1Accept data
 */
struct AcqWaveForm
{
  double         vfTrig;
  vector<double> vfTime;
  vector<double> vfVoltage;
};
typedef vector<AcqWaveForm> AcqChannelList;

static AcqChannelList                   lAcqValue       [NumAcqDetector];
static const Camera::TwoDGaussianV1*    lpTwoDGaussian  [NumFrameDetector];
static const Camera::FrameV1*           lpFrame         [NumFrameDetector];
static vector<const EpicsPvHeader*>     vpEpicsValue;
static const BldDataFEEGasDetEnergy*    pBldFeeGasDetEnergy = NULL;
static const BldDataEBeam*              pBldEBeam           = NULL;
static const BldDataEBeamV0*            pBldEBeamV0         = NULL;
static const BldDataPhaseCavity*        pBldPhaseCavity     = NULL;
static const PNCCD::FrameV1*            lpnCcdFrame     [NumPnCcdDetector];
static const EvrData::DataV3*           pEvrDataV3          = NULL;
static const Princeton::FrameV1*        lpPrincetonFrame[NumPrincetonDetector];
static const Princeton::InfoV1*         lpPrincetonInfo [NumPrincetonDetector];
static const Ipimb::DataV1*             lpIpimbData     [NumIpimbDetector];
static const Encoder::DataV1*           lpEncoderData   [NumEncoderDetector];
static const Encoder::DataV2*           lpEncoderDataV2 [NumEncoderDetector];

/*
 * Control (Calib) data
 */
static vector<unsigned char>  vcMemControlConfig;
static map<string, int>       mapControlPvNameToIndex;
static map<string, int>       mapMonitorPvNameToIndex;


static int AcqDetectorIndex(const DetInfo::Detector det, int iDevId)
{
  if (( (int) det >= DetInfo::AmoIms && (int) det <= DetInfo::Camp )) {
    // AMO
    return (int) det - DetInfo::AmoIms;
  } else if ((iDevId < 0) || (iDevId > 1)) {
    printf( "AcqDetectorIndex(): Unsupported Acq Detector %s Device Id %d\n", DetInfo::name(det), iDevId );
    return -1;
  } else if ((int) det == DetInfo::SxrBeamline) {
    // SXR
    return iDevId;      // 0, 1
  } else if ((int) det == DetInfo::SxrEndstation) {
    // SXR
    return iDevId + 2;  // 2, 3
  } else {
    printf( "AcqDetectorIndex(): Unsupported Acq Detector %s\n", DetInfo::name(det) );
    return -1;
  }
}

#define Opal1kDetectorIndex(x, y)   FrameDetectorIndex(x, DetInfo::Opal1000, y)
#define Tm6740DetectorIndex(x, y)   FrameDetectorIndex(x, DetInfo::TM6740  , y)
#define FccdDetectorIndex(x, y)     FrameDetectorIndex(x, DetInfo::Fccd, y)
static int FrameDetectorIndex(const DetInfo::Detector det, const DetInfo::Device dev, int iDevId) 
{
  if (det == DetInfo::AmoVmi || det == DetInfo::Camp) {
    return 0;
  } else if (det == DetInfo::AmoBps && iDevId < 2) {
    return iDevId+1;
  } else if (det == DetInfo::SxrBeamline) {
    if (dev == DetInfo::Opal1000 && iDevId < 2) {
      return iDevId;    // 0, 1
    }
  } else if (det == DetInfo::SxrEndstation) {
    if (dev == DetInfo::Opal1000 && iDevId < 2) {
      return iDevId+2;  // 2, 3
    } else if (dev == DetInfo::Fccd) {
      return SxrFccd;   // 4
    }
  } else if (det == DetInfo::XppSb1Pim) {
    return XppSb1PimCvd;
  } else if (det == DetInfo::XppMonPim) {
    return XppMonPimCvd;
  } else if (det == DetInfo::XppSb3Pim) {
    return XppSb3PimCvd;
  } else if (det == DetInfo::XppSb4Pim) {
    return XppSb4PimCvd;
  } else if (det == DetInfo::XppEndstation) {
    return XppEndstationCam1;
  }

  printf( "Unsupported Frame Detector %s  Device %s  DevId %d\n",
          DetInfo::name(det), DetInfo::name(dev), iDevId );
  return -1;
}

static int PrincetonDetectorIndex(const DetInfo::Detector det, int iDevId) 
{
  int iDetectorIndex = -1;
  if ( det == DetInfo::AmoEndstation )
    iDetectorIndex = iDevId + AmoPrinceton1;
  else if ( det == DetInfo::SxrBeamline )
    iDetectorIndex = iDevId + SxrBeamlinePrinceton1;
  else if ( det == DetInfo::SxrEndstation )
    iDetectorIndex = iDevId + SxrEndstationPrinceton1;
  
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumPrincetonDetector ) 
  {
    printf( "PrincetonDetectorIndex(): Unsupported Princeton Detector %s Device Id %d\n", DetInfo::name(det), iDevId );
    return -1;
  }
  
  return iDevId;
}

static int IpimbDetectorIndex(const DetInfo::Detector det, int iDevId) 
{
  if (det == DetInfo::SxrBeamline) {
    return iDevId;
  } else if (det == DetInfo::SxrEndstation) {
    return iDevId + (SxrEndstationIpimb1 - SxrBeamlineIpimb1);
  } else if (det == DetInfo::XppSb1Ipm) {
    return XppSb1Ipm;
  } else if (det == DetInfo::XppSb1Pim) {
    return XppSb1Pim;
  } else if (det == DetInfo::XppMonPim) {
    return XppMonPim;
  } else if (det == DetInfo::XppSb2Ipm) {
    return XppSb2Ipm;
  } else if (det == DetInfo::XppSb3Ipm) {
    return XppSb3Ipm;
  } else if (det == DetInfo::XppSb3Pim) {
    return XppSb3Pim;
  } else if (det == DetInfo::XppSb4Pim) {
    return XppSb4Pim;
  }

  printf( "IpimbDetectorIndex(): Unsupported Ipimb Detector %s Device Id %d\n", DetInfo::name(det), iDevId );
  return -1;
}

static int EncoderDetectorIndex(const DetInfo::Detector det, int iDevId)
{
   if (iDevId >= NumEncoderDetector) {
    printf( "EncoderDetectorIndex(): Unsupported Encoder Detector %s Device Id %d\n",
            DetInfo::name(det), iDevId );
    return -1;
  }

  return iDevId;
}

static const int NumPimDetector = 4;
static const Lusi::DiodeFexConfigV1* lDiodeFexConfig[NumPimDetector];
static const Lusi::DiodeFexV1*       lDiodeFexValue [NumPimDetector];

static int PimDetectorIndex(Pds::DetInfo::Detector det)
{
  switch(det) {
  case DetInfo::XppSb1Pim: return 0;
  case DetInfo::XppMonPim: return 1;
  case DetInfo::XppSb3Pim: return 2;
  case DetInfo::XppSb4Pim: return 3;
  default: break;
  }
  return -1;
}

static const int NumIpmDetector = 7;
static const Lusi::IpmFexConfigV1* lIpmFexConfig[NumIpmDetector];
static const Lusi::IpmFexV1*       lIpmFexValue [NumIpmDetector];

static int IpmDetectorIndex(Pds::DetInfo::Detector det)
{
  switch(det) {
  case DetInfo::XppSb1Ipm: return 0;
  case DetInfo::XppSb2Ipm: return 1;
  case DetInfo::XppSb3Ipm: return 2;
  case DetInfo::XppSb1Pim: return 3;
  case DetInfo::XppMonPim: return 4;
  case DetInfo::XppSb3Pim: return 5;
  case DetInfo::XppSb4Pim: return 6;
  default: break;
  }
  return -1;
}

static CsPad::ConfigV1         lCspadConfigV1;
static CsPad::ConfigV2         lCspadConfigV2;
static unsigned                lCspadConfigFound;
static const Xtc*              lCspadData;

static int resetStaticData()
{
  memset( lcAcqProcessed, 0, sizeof(lcAcqProcessed) );
  memset( lAcqConfig,     0, sizeof(lAcqConfig) );

  memset( lcFrameProcessed,  0, sizeof(lcFrameProcessed) );
  memset( lpOpal1kConfig,     0, sizeof(lpOpal1kConfig) );
  memset( lpTm6740ConfigV1,   0, sizeof(lpTm6740ConfigV1) );
  memset( lpTm6740Config,     0, sizeof(lpTm6740Config) );
  memset( lpFrameFexConfig,   0, sizeof(lpFrameFexConfig) );    
  memset( lFccdConfig,        0, sizeof(lFccdConfig) );
  memset( lpTwoDGaussian,     0, sizeof(lpTwoDGaussian) );
  memset( lpFrame,            0, sizeof(lpFrame) );    

  vcEpicsProcessed      .clear();
  vpEpicsConfig         .clear();
  mapEpicsPvNameToIndex .clear();
  vpEpicsValue          .clear();
  
  memset( lcPnCcdProcessed, 0, sizeof(lcPnCcdProcessed) );
  memset( lpnCcdCfg,        0, sizeof(lpnCcdCfg) );
  memset( lpnCcdFrame,      0, sizeof(lpnCcdFrame) );  

  memset( lcPrincetonProcessed, 0, sizeof(lcPrincetonProcessed) );
  memset( lprincetonCfg,        0, sizeof(lprincetonCfg) );
  memset( lpPrincetonFrame,     0, sizeof(lpPrincetonFrame) );  
  memset( lpPrincetonInfo,      0, sizeof(lpPrincetonInfo) );  

  memset( lcIpimbProcessed, 0, sizeof(lcIpimbProcessed) );
  memset( lipimbCfg,        0, sizeof(lipimbCfg) );
  memset( lpIpimbData,      0, sizeof(lpIpimbData) );

  memset( lcEncoderProcessed, 0, sizeof(lcEncoderProcessed) );
  memset( lencoderCfg,        0, sizeof(lencoderCfg) );
  memset( lpEncoderData,      0, sizeof(lpEncoderData) );
  memset( lpEncoderDataV2,    0, sizeof(lpEncoderDataV2) );

  memset( lDiodeFexConfig,    0, sizeof(lDiodeFexConfig) );
  memset( lIpmFexConfig  ,    0, sizeof(lIpmFexConfig  ) );

  lCspadConfigFound   = 0;

  bControlProcessed       = false;  
  vcMemControlConfig      .clear();
  mapControlPvNameToIndex .clear();
  mapMonitorPvNameToIndex .clear();
  
  pBldFeeGasDetEnergy = NULL;
  pBldEBeam           = NULL;
  pBldEBeamV0         = NULL;
  pBldPhaseCavity     = NULL;
  pEvrDataV3          = NULL;
  
  _fiducials          = 0;

  return 0;
}

static int resetEventData()
{
    // mark all detectors as "Not Processed"
    memset    ( lcAcqProcessed,     0, sizeof(lcAcqProcessed) );
    memset    ( lcFrameProcessed,   0, sizeof(lcFrameProcessed) );    
    std::fill ( vcEpicsProcessed.begin(), vcEpicsProcessed.end(), 0 );
    memset    ( lcPrincetonProcessed, 0, sizeof(lcPrincetonProcessed) );    
    memset    ( lcIpimbProcessed, 0, sizeof(lcIpimbProcessed) );    
    memset    ( lcEncoderProcessed, 0, sizeof(lcEncoderProcessed) );    
    memset    ( lDiodeFexValue, 0, sizeof(lDiodeFexValue) );    
    memset    ( lIpmFexValue, 0, sizeof(lIpmFexValue) );    
    
    lCspadData          = NULL;

    // Note: bControlProcessed is not reset, because it is reserved for later use
    
    pBldFeeGasDetEnergy = NULL;
    pBldEBeam           = NULL;
    pBldEBeamV0         = NULL;
    pBldPhaseCavity     = NULL;
    pEvrDataV3          = NULL;    
    
    return 0;
}

/*
 * Configure data retrieval functions
 */
int getAcqConfig(AcqDetector det, int& numChannels, int& numSamples, double& sampleInterval)
{
  int iDetectorIndex = (int) det;
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumAcqDetector )
    { printf( "getAcqConfig(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }  
  if ( lcAcqProcessed[iDetectorIndex] == 0 ) return 2; // Data has never been processed or read from xtc
    
  Acqiris::ConfigV1& acqCfg = lAcqConfig[iDetectorIndex];
  const Acqiris::HorizV1& horiz = acqCfg.horiz();
    
  numChannels = acqCfg.nbrChannels();
  numSamples = horiz.nbrSamples();
  sampleInterval = horiz.nbrSamples() * horiz.sampInterval();    
  return 0;
}
 
int getFrameConfig(FrameDetector det)
{
  int iDetectorIndex = (int) det;
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumFrameDetector )
    { printf( "getFrameConfig(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }  
  if ( lcFrameProcessed[iDetectorIndex] < 2 ) return 2;// Data has not been totally processed -> Either Opal1k::Config or FrameFexConfig is missing
    
  //const Opal1k::ConfigV1& opal1kcfg = *lpOpal1kConfig[iDetectorIndex];                
  //const Camera::FrameFexConfigV1& frameFexcfg = *lpFrameFexConfig[iDetectorIndex];    
  return 0;
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
  int iDetectorIndex = (int) det;
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumFrameDetector )
    { printf( "getFrameConfig(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }
  if ( lcFrameProcessed[iDetectorIndex] < 2 ) return 2;// Data has not been totally processed

  outputMode = lFccdConfig[iDetectorIndex].outputMode();
  ccdEnable = lFccdConfig[iDetectorIndex].ccdEnable();
  focusMode = lFccdConfig[iDetectorIndex].focusMode();
  exposureTime = lFccdConfig[iDetectorIndex].exposureTime();
  dacVoltage1 = lFccdConfig[iDetectorIndex].dacVoltage1();
  dacVoltage2 = lFccdConfig[iDetectorIndex].dacVoltage2();
  dacVoltage3 = lFccdConfig[iDetectorIndex].dacVoltage3();
  dacVoltage4 = lFccdConfig[iDetectorIndex].dacVoltage4();
  dacVoltage5 = lFccdConfig[iDetectorIndex].dacVoltage5();
  dacVoltage6 = lFccdConfig[iDetectorIndex].dacVoltage6();
  dacVoltage7 = lFccdConfig[iDetectorIndex].dacVoltage7();
  dacVoltage8 = lFccdConfig[iDetectorIndex].dacVoltage8();
  dacVoltage9 = lFccdConfig[iDetectorIndex].dacVoltage9();
  dacVoltage10 = lFccdConfig[iDetectorIndex].dacVoltage10();
  dacVoltage11 = lFccdConfig[iDetectorIndex].dacVoltage11();
  dacVoltage12 = lFccdConfig[iDetectorIndex].dacVoltage12();
  dacVoltage13 = lFccdConfig[iDetectorIndex].dacVoltage13();
  dacVoltage14 = lFccdConfig[iDetectorIndex].dacVoltage14();
  dacVoltage15 = lFccdConfig[iDetectorIndex].dacVoltage15();
  dacVoltage16 = lFccdConfig[iDetectorIndex].dacVoltage16();
  dacVoltage17 = lFccdConfig[iDetectorIndex].dacVoltage17();
  waveform0 = lFccdConfig[iDetectorIndex].waveform0();
  waveform1 = lFccdConfig[iDetectorIndex].waveform1();
  waveform2 = lFccdConfig[iDetectorIndex].waveform2();
  waveform3 = lFccdConfig[iDetectorIndex].waveform3();
  waveform4 = lFccdConfig[iDetectorIndex].waveform4();
  waveform5 = lFccdConfig[iDetectorIndex].waveform5();
  waveform6 = lFccdConfig[iDetectorIndex].waveform6();
  waveform7 = lFccdConfig[iDetectorIndex].waveform7();
  waveform8 = lFccdConfig[iDetectorIndex].waveform8();
  waveform9 = lFccdConfig[iDetectorIndex].waveform9();
  waveform10 = lFccdConfig[iDetectorIndex].waveform10();
  waveform11 = lFccdConfig[iDetectorIndex].waveform11();
  waveform12 = lFccdConfig[iDetectorIndex].waveform12();
  waveform13 = lFccdConfig[iDetectorIndex].waveform13();
  waveform14 = lFccdConfig[iDetectorIndex].waveform14();

  return 0;
}

int getDiodeFexConfig (Pds::DetInfo::Detector det, int iDevId, float* base, float* scale)
{
  int iDetectorIndex = PimDetectorIndex(det);
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumPimDetector )
    { printf( "getDiodeFexConfig(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }

  const Lusi::DiodeFexConfigV1* p = lDiodeFexConfig[iDetectorIndex];
  if ( p == 0) return 2;// Data has not been totally processed

  for(unsigned i=0; i<Pds::Lusi::DiodeFexConfigV1::NRANGES; i++) {
    base [i] = p->base [i];
    scale[i] = p->scale[i];
  }
  return 0;
}

int getIpmFexConfig   (Pds::DetInfo::Detector det, int iDevId, 
           float* base0, float* scale0,
           float* base1, float* scale1,
           float* base2, float* scale2,
           float* base3, float* scale3,
           float& xscale, float& yscale)
{
  int iDetectorIndex = IpmDetectorIndex(det);
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumIpmDetector )
    { printf( "getIpmFexConfig(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }

  const Lusi::IpmFexConfigV1* p = lIpmFexConfig[iDetectorIndex];
  if ( p == 0 ) return 2;// Data has not been totally processed

  float* barray[] = { base0 , base1 , base2 , base3  };
  float* sarray[] = { scale0, scale1, scale2, scale3 };
  for(unsigned j=0; j<Pds::Lusi::IpmFexConfigV1::NCHANNELS; j++) {
    const Lusi::DiodeFexConfigV1& c = p->diode[j];
    float* base  = barray[j];
    float* scale = sarray[j];
    for(unsigned i=0; i<Pds::Lusi::DiodeFexConfigV1::NRANGES; i++) {
      base [i] = c.base [i];
      scale[i] = c.scale[i];
    }
  }
  xscale = p->xscale;
  yscale = p->yscale;

  return 0;
}


int getCspadConfig (Pds::DetInfo::Detector det, Pds::CsPad::ConfigV1& cfg)
{
  if ( det != Pds::DetInfo::XppGon )
    { printf( "getCspadConfig(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }

  if ( lCspadConfigFound != 1 ) return 2;// Data has not been totally processed

  cfg = lCspadConfigV1;

  return 0;
}


int getCspadConfig (Pds::DetInfo::Detector det, Pds::CsPad::ConfigV2& cfg)
{
  if ( det != Pds::DetInfo::XppGon )
    { printf( "getCspadConfig(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }

  if ( lCspadConfigFound != 2 ) return 2;// Data has not been totally processed

  cfg = lCspadConfigV2;
  return 0;
}


int getEpicsPvNumber()
{
  return vcEpicsProcessed.size();
}

int getEpicsPvConfig( int pvId, const char*& pvName, int& type, int& numElements )
{
  if ( pvId < 0 || pvId >= (int) vpEpicsConfig.size() ) 
    { printf( "getEpicsPvConfig(): PV Id (%d) is invalid.\n", pvId ); return 1; }  
    
  const EpicsPvCtrlHeader& epicsPv = *static_cast<const EpicsPvCtrlHeader*>( vpEpicsConfig[ pvId ] );
  pvName = epicsPv.sPvName;
  type = epicsPv.iDbrType - DBR_CTRL_DOUBLE + DBR_DOUBLE; // convert from ctrl type to basic type
  numElements = epicsPv.iNumElements;
    
  return 0;
}

int getPrincetonConfig(DetInfo::Detector det, int iDevId, int& width, int& height, int& orgX, int& orgY, int& binX, int&binY)
{
  int iDetectorIndex = PrincetonDetectorIndex(det, iDevId);
  if (iDetectorIndex < 0) {
    return 1;
  }  
  if ( lcPrincetonProcessed[iDetectorIndex] == 0 ) return 2;  
    
  const Princeton::ConfigV1& princetonCfg = lprincetonCfg[iDetectorIndex];                
  
  width   = princetonCfg.width  ();
  height  = princetonCfg.height ();
  orgX    = princetonCfg.orgX   ();
  orgY    = princetonCfg.orgY   ();
  binX    = princetonCfg.binX   ();
  binY    = princetonCfg.binY   ();

  return 0;
}

//
// Configuration includes chargeAmpRange for channels 0-3.
// Values: 0=high gain, 1=medium gain, 2=low gain
//
int getIpimbConfig(DetInfo::Detector det, int iDevId, uint64_t& serialID,
                   int& chargeAmpRange0, int& chargeAmpRange1,
                   int& chargeAmpRange2, int& chargeAmpRange3)
{
  int iDetectorIndex = IpimbDetectorIndex(det, iDevId);
  if (iDetectorIndex < 0) {
    return 1;
  }  
  if ( lcIpimbProcessed[iDetectorIndex] == 0 ) {
    return 2;  
  }
  serialID = lipimbCfg[iDetectorIndex].serialID();
  chargeAmpRange0 = lipimbCfg[iDetectorIndex].chargeAmpRange() & 0x3;
  chargeAmpRange1 = (lipimbCfg[iDetectorIndex].chargeAmpRange() >> 2) & 0x3;
  chargeAmpRange2 = (lipimbCfg[iDetectorIndex].chargeAmpRange() >> 4) & 0x3;
  chargeAmpRange3 = (lipimbCfg[iDetectorIndex].chargeAmpRange() >> 6) & 0x3;
  return 0;
}

int getEncoderConfig(DetInfo::Detector det, int iDevId)
{
  int iDetectorIndex = EncoderDetectorIndex(det, iDevId);
  if (iDetectorIndex < 0) {
    return 1;
  } 
  if ( lcEncoderProcessed[iDetectorIndex] == 0 ) {
    return 2;
  }

  return 0;
}

/*
 * L1Accept data retrieval functions
 */
int getEvrDataNumber()
{
  if ( pEvrDataV3 == NULL ) return 0;
  
  return pEvrDataV3->numFifoEvents();
}

int getEvrData( int id, unsigned int& eventCode, unsigned int& fiducial, unsigned int& timeStamp )
{
  if ( pEvrDataV3 == NULL ) 
    return 1;
  if ( id < 0 || 
       id >= (int) pEvrDataV3->numFifoEvents() )
    return 2;
  
  const EvrData::DataV3::FIFOEvent& fifoEvent = pEvrDataV3->fifoEvent(id);
  
  eventCode = fifoEvent.EventCode;
  fiducial  = fifoEvent.TimestampHigh;
  timeStamp = fifoEvent.TimestampLow;
  
  return 0;
}

 int getAcqValue(AcqDetector det, int channel, double*& time, double*& voltage, double& trigtime)
{
  int iDetectorIndex = (int) det;
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumAcqDetector )
    { printf( "getAcqValue(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }  
  if ( lcAcqProcessed[iDetectorIndex] == 0 ) return 2; // Data has never been processed or read from xtc

  AcqChannelList& acqChannels = lAcqValue[iDetectorIndex];
    
  if ( channel < 0 || channel >= (int) acqChannels.size() ) return 3; // No such channel 
  AcqWaveForm& waveForm  = acqChannels[channel];
    
  time     = &waveForm.vfTime[0];
  voltage  = &waveForm.vfVoltage[0];
  trigtime =  waveForm.vfTrig;
    
  return 0;
}

int getAcqValue(AcqDetector det, int channel, double*& time, double*& voltage)
{ double trigtime; return getAcqValue(det, channel, time, voltage, trigtime); }

int getFrameValue(FrameDetector det, int& frameWidth, int& frameHeight, unsigned short*& image )
{
  int iDetectorIndex = (int) det;
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumFrameDetector )
    { printf( "getFrameValue(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }     
  if ( lcFrameProcessed[iDetectorIndex] < 2 ) return 2; // Data has not been totally processed -> Either TwoDGaussian or Frame is missing
    
  //const Camera::TwoDGaussianV1& twoDGaussian = *lpTwoDGaussian[iDetectorIndex];       
  const Camera::FrameV1& frame = *lpFrame[iDetectorIndex];
  frameWidth  = frame.width();
  frameHeight = frame.height();
  image       = (unsigned short*) frame.data();
  return 0;
}

int getPrincetonValue(Pds::DetInfo::Detector det, int iDevId, unsigned short *& image)
{
  int iDetectorIndex = PrincetonDetectorIndex(det, iDevId);
  if (iDetectorIndex < 0) {
    return 1;
  }  
  if ( lcPrincetonProcessed[iDetectorIndex] == 0 ) return 2; 
    
  const Princeton::ConfigV1& princetonCfg = lprincetonCfg[iDetectorIndex];
  if ( princetonCfg.delayMode() != 0 )
  {
    printf( "getPrincetonValue(): Delay mode is not supported by myana\n" );
    return 3;
  }  
  
  const Princeton::FrameV1& princetonFrame = *lpPrincetonFrame[iDetectorIndex];                
  image = (unsigned short*) princetonFrame.data();
  
  return 0;
}

int getPrincetonTemperature(Pds::DetInfo::Detector det, int iDevId, float & fTemperature)
{
  int iDetectorIndex = PrincetonDetectorIndex(det, iDevId);
  if (iDetectorIndex < 0) {
    return 1;
  }  
  if ( lcPrincetonProcessed[iDetectorIndex] == 0 ) return 2; 
  
  // Princeton Frame data is available, but no princeton info
  //   Not a valid case now, but can happen in the future if info is recorded less frequently than frame data.
  if ( lpPrincetonInfo[iDetectorIndex] == NULL )   return 3; 
      
  const Princeton::InfoV1& princetonInfo = *lpPrincetonInfo[iDetectorIndex];
  
  fTemperature = princetonInfo.temperature();
  
  return 0;
}

int getIpimbVolts(Pds::DetInfo::Detector det, int iDevId,
                  float &channel0, float &channel1, float &channel2, float &channel3)
{
  int iDetectorIndex = IpimbDetectorIndex(det, iDevId);
  if (iDetectorIndex < 0) {
    return 1;
  }  
  if ( lcIpimbProcessed[iDetectorIndex] == 0 ) {
    return 2; 
  }
    
  const Ipimb::DataV1& ipimbData = *lpIpimbData[iDetectorIndex];                
  channel0 = ipimbData.channel0Volts();
  channel1 = ipimbData.channel1Volts();
  channel2 = ipimbData.channel2Volts();
  channel3 = ipimbData.channel3Volts();
  
  return 0;
}

int getEncoderCount(Pds::DetInfo::Detector det, int iDevId, int& encoderCount, int chan)
{
  int iDetectorIndex = EncoderDetectorIndex(det, iDevId);
  if (iDetectorIndex < 0) {
    return 1;
  }  
  if ( lcEncoderProcessed[iDetectorIndex] == 0 ) {
    return 2; 
  }
  if (lpEncoderDataV2[iDetectorIndex] != 0){
    encoderCount = lpEncoderDataV2[iDetectorIndex]->value(chan);
  }
  else {
    encoderCount = lpEncoderData[iDetectorIndex]->value();
  }
  return 0;
}

int getDiodeFexValue (Pds::DetInfo::Detector det, int iDevId, float& value)
{
  int iDetectorIndex = PimDetectorIndex(det);
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumPimDetector )
    { printf( "getDiodeFexValue(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }

  const Lusi::DiodeFexV1* p = lDiodeFexValue[iDetectorIndex];
  if ( p == 0) return 2;// Data has not been totally processed

  value = p->value;
  return 0;
}

int getIpmFexValue   (Pds::DetInfo::Detector det, int iDevId, 
          float* channels, float& sum, float& xpos, float& ypos)
{
  int iDetectorIndex = IpmDetectorIndex(det);
  if ( iDetectorIndex < 0 || iDetectorIndex >= NumPimDetector )
    { printf( "getIpmFexValue(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }

  const Lusi::IpmFexV1* p = lIpmFexValue[iDetectorIndex];
  if ( p == 0) return 2;// Data has not been totally processed

  for(unsigned i=0; i<Lusi::IpmFexConfigV1::NCHANNELS; i++) 
    channels[i] = p->channel[i];
  sum  = p->sum;
  xpos = p->xpos;
  ypos = p->ypos;

  return 0;
}

int getCspadData  (Pds::DetInfo::Detector det, Pds::CsPad::ElementIterator& iter)
{
  if ( det != Pds::DetInfo::XppGon )
    { printf( "getCspadQuad(): Detector Id (%d) is out of range.\n", (int) det ); return 1; }

  if (lCspadData) {
    if (lCspadConfigFound==1)
      iter = Pds::CsPad::ElementIterator(lCspadConfigV1, *lCspadData);
    else if (lCspadConfigFound==2)
      iter = Pds::CsPad::ElementIterator(lCspadConfigV2, *lCspadData);
    else
      return 3;
  }
  else
    return 2;

  return 0;
}

int getFeeGasDet(double* shotEnergy)
{
  if ( pBldFeeGasDetEnergy == NULL ) return 1;
  
  shotEnergy[0] = pBldFeeGasDetEnergy->f_11_ENRC;
  shotEnergy[1] = pBldFeeGasDetEnergy->f_12_ENRC;
  shotEnergy[2] = pBldFeeGasDetEnergy->f_21_ENRC;
  shotEnergy[3] = pBldFeeGasDetEnergy->f_22_ENRC;
  return 0;
}

int getPhaseCavity(double& fitTime1, double& fitTime2,
                   double& charge1,  double& charge2)
{
  if ( pBldPhaseCavity == NULL ) return 1;
  
  fitTime1 = pBldPhaseCavity->fFitTime1;
  fitTime2 = pBldPhaseCavity->fFitTime2;
  charge1  = pBldPhaseCavity->fCharge1;
  charge2  = pBldPhaseCavity->fCharge2;
  return 0;
}

int getEBeam(double& charge, double& energy, double& posx, double& posy,
             double& angx, double& angy, double& pkcurr)
{
  if ( pBldEBeam == NULL ) return 1;
  
  charge = pBldEBeam->fEbeamCharge;    /* in nC */ 
  energy = pBldEBeam->fEbeamL3Energy;  /* in MeV */ 
  posx   = pBldEBeam->fEbeamLTUPosX;   /* in mm */ 
  posy   = pBldEBeam->fEbeamLTUPosY;   /* in mm */ 
  angx   = pBldEBeam->fEbeamLTUAngX;   /* in mrad */ 
  angy   = pBldEBeam->fEbeamLTUAngY;   /* in mrad */  
  pkcurr = pBldEBeam->fEbeamPkCurrBC2; /* in Amps */
  return 0;
}

int getEBeam(double& charge, double& energy, double& posx, double& posy,
             double& angx, double& angy)
{
  double pkc; 
  if ( getEBeam(charge, energy, posx, posy, angx, angy, pkc) == 0 )
    return 0;
    
  if ( pBldEBeamV0 == NULL ) return 1;
  
  charge = pBldEBeamV0->fEbeamCharge;    /* in nC */ 
  energy = pBldEBeamV0->fEbeamL3Energy;  /* in MeV */ 
  posx   = pBldEBeamV0->fEbeamLTUPosX;   /* in mm */ 
  posy   = pBldEBeamV0->fEbeamLTUPosY;   /* in mm */ 
  angx   = pBldEBeamV0->fEbeamLTUAngX;   /* in mrad */ 
  angy   = pBldEBeamV0->fEbeamLTUAngY;   /* in mrad */  
  return 0;
}

template <int iDbrId> 
static int getEpicsPvValueByType( const EpicsPvHeader& epicsPv1, const void** ppValue, int* piDbrype, struct tm* pTmTimeStamp, int* piNanoSec )
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

typedef int (*TEpicValueFuncPtr)(const EpicsPvHeader& epicsPv1, const void** ppValue, int* piDbrype, struct tm* pTmTimeStamp, int* piNanoSec);
TEpicValueFuncPtr lpfEpicsValueFunc[] = 
  {
    &getEpicsPvValueByType<DBR_STRING>, &getEpicsPvValueByType<DBR_SHORT>,  &getEpicsPvValueByType<DBR_FLOAT>,
    &getEpicsPvValueByType<DBR_ENUM>,   &getEpicsPvValueByType<DBR_CHAR>,   &getEpicsPvValueByType<DBR_LONG>,
    &getEpicsPvValueByType<DBR_DOUBLE>
  };

int getEpicsPvValue( int pvId, const void*& value, int& dbrype, struct tm& tmTimeStamp, int& nanoSec )
{
  if ( pvId < 0 || pvId >= (int) vpEpicsValue.size() )
    { printf( "getEpicsPvConfig(): PV Id (%d) is invalid.\n", pvId ); return 1; }    
  if ( vcEpicsProcessed[pvId] == 0 ) return 2;// Data has never been processed or read from xtc    

  const EpicsPvHeader& epicsPv = *vpEpicsValue[ pvId ];    
    
  static const int iBasicType = epicsPv.iDbrType - DBR_TIME_DOUBLE + DBR_DOUBLE;    
  if ( iBasicType < 0 || iBasicType > DBR_DOUBLE ) // Not a valid PV type
    { printf( "getEpicsPvValue(): Data type for PV Id %d is not valid.\n", pvId ); return 3; }     
    
  return ( *lpfEpicsValueFunc[ iBasicType ] ) ( epicsPv, &value, &dbrype, &tmTimeStamp, &nanoSec );
}

int getPvInt(const char* pvName, int& value)
{
  if ( mapEpicsPvNameToIndex.find( pvName ) == mapEpicsPvNameToIndex.end() ) return 1; // No such Pv
  int iPvId = mapEpicsPvNameToIndex[pvName];
    
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
  if ( mapEpicsPvNameToIndex.find( pvName ) == mapEpicsPvNameToIndex.end() ) return 1; // No such Pv    
  int iPvId = mapEpicsPvNameToIndex[pvName];
    
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
      printf( "getPvInt(%s): Not an floating point value PV.\n", pvName );    
      return 3; // Unsupported iDbrType
    }    
    
  return 0;    
}

int getPvString(const char* pvName, char*& value)
{
  if ( mapEpicsPvNameToIndex.find( pvName ) == mapEpicsPvNameToIndex.end() ) return 1; // No such Pv
  int iPvId = mapEpicsPvNameToIndex[pvName];
    
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

int getPnCcdValue( int deviceId, unsigned char*& image, int& width, int& height )
{
  if ( deviceId < 0 || deviceId > 1)
    { printf( "getPnCcdValue(): pnCCD device Id (%d) is out of range (0..1)\n", deviceId ); return 1; }
  if ( lcPnCcdProcessed[deviceId] == 0 ) return 2;// Data doesn't exist

  const PNCCD::ConfigV1&  config  = lpnCcdCfg[deviceId];  
  const PNCCD::FrameV1*   pFrame0 = lpnCcdFrame[deviceId];
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
  if ( !bControlProcessed || vcMemControlConfig.size() == 0 )
    return 0;
 
  const ControlData::ConfigV1& config = *(const ControlData::ConfigV1*) &vcMemControlConfig[0];
  return config.npvControls();
}

int getControlPvName( int pvId, const char*& pvName, int& arrayIndex )
{
  if ( !bControlProcessed || vcMemControlConfig.size() == 0 )
    return 1;

  const ControlData::ConfigV1& config = *(const ControlData::ConfigV1*) &vcMemControlConfig[0];
  if ( pvId < 0 || pvId >= (int) config.npvControls() ) // no such pvId
    return 2;
    
  const Pds::ControlData::PVControl& pvControlCur = config.pvControl(pvId);
  pvName = pvControlCur.name();

  if (pvControlCur.array())
    arrayIndex = pvControlCur.index();
  else
    arrayIndex = 0;
    
  return 0;
}

int getControlValue(const char* pvName, int arrayIndex, double& value )
{
  if ( !bControlProcessed || vcMemControlConfig.size() == 0 )
    return 1;
    
  const ControlData::ConfigV1& config = *(const ControlData::ConfigV1*) &vcMemControlConfig[0];

  string pvHashKey = GenerateCtrlPvHashKey(pvName, arrayIndex);
  if ( mapControlPvNameToIndex.find( pvHashKey ) == mapControlPvNameToIndex.end() ) // No such Pv
    return 1; 
    
  int iPvId = mapEpicsPvNameToIndex[pvHashKey];      
  if ( iPvId < 0 || iPvId >= (int) config.npvControls() ) // no such pvId
    return 2;
    
  const Pds::ControlData::PVControl& pvControlCur = config.pvControl(iPvId);
  value = pvControlCur.value();
    
  return 0;
}

int getMonitorPvNumber()
{
  if ( !bControlProcessed || vcMemControlConfig.size() == 0 )
    return 0;
 
  const ControlData::ConfigV1& config = *(const ControlData::ConfigV1*) &vcMemControlConfig[0];
  return config.npvMonitors();
}

int getMonitorPvName( int pvId, const char*& pvName, int& arrayIndex )
{
  if ( !bControlProcessed || vcMemControlConfig.size() == 0 )
    return 1;

  const ControlData::ConfigV1& config = *(const ControlData::ConfigV1*) &vcMemControlConfig[0];
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
  if ( !bControlProcessed || vcMemControlConfig.size() == 0 )
    return 1;
    
  const ControlData::ConfigV1& config = *(const ControlData::ConfigV1*) &vcMemControlConfig[0];

  string pvHashKey = GenerateCtrlPvHashKey(pvName, arrayIndex);
  if ( mapMonitorPvNameToIndex.find( pvHashKey ) == mapMonitorPvNameToIndex.end() ) // No such Pv
    return 1; 
    
  int iPvId = mapEpicsPvNameToIndex[pvHashKey];      
  if ( iPvId < 0 || iPvId >= (int) config.npvMonitors() || iPvId >= (int) config.npvMonitors() ) // no such pvId
    return 2;
    
  const Pds::ControlData::PVMonitor& pvMonitorCur = config.pvMonitor(iPvId);
          
  hilimit = pvMonitorCur.hiValue();
  lolimit = pvMonitorCur.loValue();
    
  return 0;
}

class myLevelIter : public XtcIterator 
{
public:
  enum {Stop, Continue};
  myLevelIter(Xtc* xtc, unsigned depth, int iDebugLevel) : XtcIterator(xtc), _depth(depth), _iDebugLevel(iDebugLevel) {}

  void process(const DetInfo& di, const Acqiris::ConfigV1& cfg) 
  {
    int iDetectorIndex = AcqDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    lAcqConfig[iDetectorIndex]=cfg;
    lcAcqProcessed[iDetectorIndex] = 1;
  }

  void process(const DetInfo& di, Acqiris::DataDescV1& datadesc) 
  {
    int iDetectorIndex = AcqDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    const Acqiris::ConfigV1& acqCfg = lAcqConfig[iDetectorIndex];
    const Acqiris::HorizV1& hcfg = acqCfg.horiz();
    double sampInterval = hcfg.sampInterval();
    Acqiris::DataDescV1* ddesc = &datadesc;
        
    AcqChannelList& acqChannels = lAcqValue[iDetectorIndex];
        
    const int iNumChannels = acqCfg.nbrChannels();
    if ( iNumChannels > (int) acqChannels.size() ) acqChannels.resize(iNumChannels);
        
    for (unsigned i=0;i<acqCfg.nbrChannels();i++) 
      {
        const int16_t* data = ddesc->waveform(hcfg);
        data += ddesc->indexFirstPoint();
        const Acqiris::VertV1& vcfg = acqCfg.vert(i);
        float slope = vcfg.slope();
        float offset = vcfg.offset();
        int nbrSamples = hcfg.nbrSamples();
            
        AcqWaveForm& waveForm = acqChannels[i];
        if ( nbrSamples > (int) waveForm.vfTime.size() )
          { // grow the size of vtTime and vfVoltage at the same time
            waveForm.vfTime.resize(nbrSamples);
            waveForm.vfVoltage.resize(nbrSamples);
          }
           
        waveForm.vfTrig = ddesc->timestamp(0).pos(); 
        for (int j=0;j<nbrSamples;j++) 
          {
            int16_t swap = (data[j]&0xff<<8) | (data[j]&0xff00>>8);
            waveForm.vfTime[j] = j*sampInterval + waveForm.vfTrig;  //time vector with horPos Offset
            waveForm.vfVoltage[j] = swap*slope-offset;
          }
        ddesc = ddesc->nextChannel(hcfg);
      }
        
    lcAcqProcessed[iDetectorIndex] = 1;
  }

  void process(const DetInfo&, const BldDataFEEGasDetEnergy& bldData) 
  {
    pBldFeeGasDetEnergy = &bldData;
  }

  void process(const DetInfo&, const BldDataEBeam& bldData) 
  {
    pBldEBeam = &bldData;
  }

  void process(const DetInfo&, const BldDataEBeamV0& bldData) 
  {
    pBldEBeamV0 = &bldData;
  }

  void process(const DetInfo&, const BldDataPhaseCavity& bldData) 
  {
    pBldPhaseCavity = &bldData;
  }

  void process(const DetInfo& di, const Opal1k::ConfigV1& cfg) 
  {
    int iDetectorIndex = Opal1kDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    lpOpal1kConfig[iDetectorIndex] = &cfg;
    lcFrameProcessed[iDetectorIndex]++;
  }
    
  void process(const DetInfo& di, const Pulnix::TM6740ConfigV1& cfg) 
  {
    int iDetectorIndex = Tm6740DetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    lpTm6740ConfigV1[iDetectorIndex] = &cfg;
    lcFrameProcessed[iDetectorIndex]++;
  }
    
  void process(const DetInfo& di, const Pulnix::TM6740ConfigV2& cfg) 
  {
    int iDetectorIndex = Tm6740DetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    lpTm6740Config  [iDetectorIndex] = &cfg;
    lcFrameProcessed[iDetectorIndex]++;
  }
    
  void process(const DetInfo& di, const Camera::FrameFexConfigV1& cfg) 
  {
    int iDetectorIndex = Opal1kDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    lpFrameFexConfig[iDetectorIndex] = &cfg;
    lcFrameProcessed[iDetectorIndex]++;        
  }

  void process(const DetInfo& di, const Camera::TwoDGaussianV1& twoDGaussian) 
  {
    int iDetectorIndex = Opal1kDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    lpTwoDGaussian[iDetectorIndex]=&twoDGaussian;
    lcFrameProcessed[iDetectorIndex]++;        
  }

  void process(const DetInfo& di, const Camera::FrameV1& frame) 
  {
    int iDetectorIndex = FrameDetectorIndex(di.detector(), di.device(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    lpFrame[iDetectorIndex]=&frame;
    lcFrameProcessed[iDetectorIndex] += 2;
  }

  void process(const DetInfo& di, const FCCD::FccdConfigV2& cfg)
  {
    int iDetectorIndex = FccdDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    lFccdConfig[iDetectorIndex] = cfg;
    lcFrameProcessed[iDetectorIndex] += 2;
  }

  void process(const DetInfo& di, const EpicsPvHeader& epicsPv) 
  {
    if ( dbr_type_is_CTRL(epicsPv.iDbrType) )
      {
        if ( epicsPv.iPvId < (int) vpEpicsConfig.size() )
          printf( "myLevelIter::process(): epics control data (id %d) is duplicated\n", epicsPv.iPvId );
        else
          vpEpicsConfig.resize( epicsPv.iPvId+1 );
        vpEpicsConfig[ epicsPv.iPvId ] = &epicsPv;
            
        mapEpicsPvNameToIndex[ ((const EpicsPvCtrlHeader&)epicsPv).sPvName ] = epicsPv.iPvId;            
      }
    else if ( dbr_type_is_TIME(epicsPv.iDbrType) )
      {
        if ( epicsPv.iPvId >= (int) vpEpicsValue.size() ) vpEpicsValue.resize( epicsPv.iPvId+1 );
        vpEpicsValue[ epicsPv.iPvId ] = &epicsPv;
      }
                
    if ( epicsPv.iPvId >= (int) vcEpicsProcessed.size() ) vcEpicsProcessed.resize( epicsPv.iPvId+1 );        
    vcEpicsProcessed[epicsPv.iPvId] = 1;
  }
   
  void process(const DetInfo& det, const PNCCD::ConfigV1& config) {
    if ( det.detId() != 0 ) 
      { printf( "myLevelIter::process(...,PNCCD::ConfigV1&): pnCCD detector Id (%d) is not 0\n", det.detId() ); return;}
    if ( det.devId() < 0 || det.devId() > 1)
      { printf( "myLevelIter::process(...,PNCCD::ConfigV1&): pnCCD device Id (%d) is out of range (0..1)\n", det.devId() ); return; }
    if ( (int) config.numLinks() != iPnCcdNumLinks )
      { printf( "myLevelIter::process(...,PNCCD::ConfigV1&): Number of links (%d) != expected value %d\n", config.numLinks(), iPnCcdNumLinks); return; }
    if ( (int) config.payloadSizePerLink() != iPnCcdPayloadSize )
      { printf( "myLevelIter::process(...,PNCCD::ConfigV1&): Payload size (%d) != expected value %d\n", config.payloadSizePerLink(), iPnCcdPayloadSize); return; }
    
    lpnCcdCfg[det.devId()] = config;
    lcPnCcdProcessed[det.devId()] = 1;
  }  
  
  void process(const DetInfo& det, const PNCCD::FrameV1* f) 
  {
    if ( det.detId() != 0 )
      { printf( "myLevelIter::process(...,PNCCD::FrameV1*): pnCCD detector Id (%d) is not 0\n", det.detId() ); return; }
    if ( det.devId() < 0 || det.devId() > 1)
      { printf( "myLevelIter::process(...,PNCCD::FrameV1*): pnCCD device Id (%d) is out of range (0..1)\n", det.devId() ); return; }
    
    lpnCcdFrame[det.devId()] = f;
    lcPnCcdProcessed[det.devId()] = 1;
  }  
  
  void process(const DetInfo&, const ControlData::ConfigV1& configOrg ) 
  {    
    vcMemControlConfig.resize( configOrg.size() );
    const ControlData::ConfigV1& config = *new ( &vcMemControlConfig[0] ) ControlData::ConfigV1(configOrg);
    
    for(unsigned int iPvControl=0; iPvControl < config.npvControls(); iPvControl++) 
    {
      const Pds::ControlData::PVControl& pvControlCur = config.pvControl(iPvControl);      
      mapControlPvNameToIndex[ GenerateCtrlPvHashKey( pvControlCur.name(), pvControlCur.index() ) ] = iPvControl;
    }

    for(unsigned int iPvMonitor=0; iPvMonitor < config.npvMonitors(); iPvMonitor++) 
    {      
      const Pds::ControlData::PVMonitor& pvMonitorCur = config.pvMonitor(iPvMonitor);
      mapMonitorPvNameToIndex[ GenerateCtrlPvHashKey( pvMonitorCur.name(), pvMonitorCur.index() ) ] = iPvMonitor;
    }
    
    bControlProcessed = true;
  }  
  
  void process(const DetInfo&, const EvrData::ConfigV1&) 
  {
    return;
  }
  
  void process(const DetInfo&, const EvrData::ConfigV2&) 
  {
    return;
  }
  void process(const DetInfo&, const EvrData::ConfigV3&) 
  {
    return;
  }
  void process(const DetInfo&, const EvrData::ConfigV4&)
  {
    return;
  }
  void process(const DetInfo&, const EvrData::DataV3& data) 
  {
    pEvrDataV3 = &data;
  }  
  
  void process(const DetInfo& di, const Princeton::ConfigV1& cfg) 
  {
    int iDetectorIndex = PrincetonDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    
    lprincetonCfg[iDetectorIndex] = cfg;
    lcPrincetonProcessed[iDetectorIndex] = 1;
  }

  void process(const DetInfo& di, const Princeton::FrameV1* frame) 
  {
    int iDetectorIndex = PrincetonDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    
    lpPrincetonFrame[iDetectorIndex] = frame;
    lcPrincetonProcessed[iDetectorIndex] = 1;
  }
  

  void process(const DetInfo& di, const Princeton::InfoV1* info) 
  {
    int iDetectorIndex = PrincetonDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    
    lpPrincetonInfo[iDetectorIndex] = info;
  }
  
  void process(const DetInfo& di, const Ipimb::ConfigV1& cfg) 
  {
    int iDetectorIndex = IpimbDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) {
      return;
    }
    lipimbCfg[iDetectorIndex] = cfg;
    lcIpimbProcessed[iDetectorIndex] = 1;
  }

  void process(const DetInfo& di, const Ipimb::DataV1* data) 
  {
    int iDetectorIndex = IpimbDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) return;
    
    lpIpimbData[iDetectorIndex] = data;
    lcIpimbProcessed[iDetectorIndex] = 1;
  }

  void process(const DetInfo& di, const CsPad::ConfigV1& cfg) 
  {
    //    if ( di.detector() == DetInfo::XppGon )
      lCspadConfigFound = 1;
      lCspadConfigV1 = cfg;

      // hack to correct unregistered version change
      // one word was added at beginning of configuration ('concentratorVersion')
      if (cfg.payloadSize()==0)
        memcpy(reinterpret_cast<char*>(&lCspadConfigV1)+4, &cfg, sizeof(cfg)-4);
  }

  void process(const DetInfo& di, const CsPad::ConfigV2& cfg) 
  {
    //    if ( di.detector() == DetInfo::XppGon )
      lCspadConfigFound = 2;
      lCspadConfigV2 = cfg;
  }

  void process(const DetInfo& di, const Encoder::ConfigV1& cfg)
  {
    int iDetectorIndex = EncoderDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) {
      return;
    }
    lencoderCfg[iDetectorIndex] = cfg;
    lcEncoderProcessed[iDetectorIndex] = 1;
  }

  void process(const DetInfo& di, const Encoder::DataV1* data)
  {
    int iDetectorIndex = EncoderDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) {
      return;
    }
    lpEncoderData[iDetectorIndex] = data;
    lcEncoderProcessed[iDetectorIndex] = 1;
  }

  void process(const DetInfo& di, const Encoder::DataV2* data)
  {
    int iDetectorIndex = EncoderDetectorIndex(di.detector(), di.devId());
    if ( iDetectorIndex < 0 ) {
      return;
    }
    lpEncoderDataV2[iDetectorIndex] = data;
    lcEncoderProcessed[iDetectorIndex] = 1;
  }

  void process(const DetInfo& di, const Lusi::DiodeFexConfigV1* data)
  {
    int iDetectorIndex = PimDetectorIndex(di.detector());
    if ( iDetectorIndex < 0 ) {
      return;
    }
    lDiodeFexConfig[iDetectorIndex] = data;
  }

  void process(const DetInfo& di, const Lusi::DiodeFexV1* data)
  {
    int iDetectorIndex = PimDetectorIndex(di.detector());
    if ( iDetectorIndex < 0 ) {
      return;
    }
    lDiodeFexValue[iDetectorIndex] = data;
  }

  void process(const DetInfo& di, const Lusi::IpmFexConfigV1* data)
  {
    int iDetectorIndex = IpmDetectorIndex(di.detector());
    if ( iDetectorIndex < 0 ) {
      return;
    }
    lIpmFexConfig[iDetectorIndex] = data;
  }

  void process(const DetInfo& di, const Lusi::IpmFexV1* data)
  {
    int iDetectorIndex = IpmDetectorIndex(di.detector());
    if ( iDetectorIndex < 0 ) {
      return;
    }
    lIpmFexValue[iDetectorIndex] = data;
  }

  
  int process(Xtc* xtc) 
  {
    if (xtc->extent        <  sizeof(Xtc) ||        // check for corrupt data
  xtc->contains.id() >= TypeId::NumberOf) {
      return Stop;
    }
    else if (xtc->contains.id() == TypeId::Id_Xtc) { // iterate through hierarchy
      myLevelIter iter(xtc,_depth+1, _iDebugLevel);
      iter.iterate();
    }
    else if (xtc->damage.value() != 0) {               // skip damaged detector data
    }
    else {                                           // handle specific detector data
      const DetInfo& info = *(DetInfo*)(&xtc->src);
      if (_iDebugLevel >= 1 && xtc->contains.id() != TypeId::Id_Xtc)
  printf( "> Xtc type %-16s  Detector %-12s:%d  Device %-12s:%d\n", TypeId::name(xtc->contains.id()),
    DetInfo::name(info.detector()), info.detId(), DetInfo::name(info.device()), info.devId() );
      switch (xtc->contains.id()) 
  {
  case (TypeId::Id_FEEGasDetEnergy) :
    {
      process(info, *(const BldDataFEEGasDetEnergy*) xtc->payload() );
      break;
    }
  case (TypeId::Id_EBeam) :
    {
      switch(xtc->contains.version()) {
      case 0 : process(info, *(const BldDataEBeamV0*) xtc->payload() ); break;
      case 1 : process(info, *(const BldDataEBeam*  ) xtc->payload() ); break;
      default: break;
      }          
      break;
    }
  case (TypeId::Id_PhaseCavity) :
    {
      process(info, *(const BldDataPhaseCavity*) xtc->payload() );
      break;
    }
  case (TypeId::Id_AcqConfig) :
    {
      unsigned version = xtc->contains.version();
      switch (version) 
        {
        case 1:
    if (info.device()==Pds::DetInfo::Acqiris)
      process(info,*(const Acqiris::ConfigV1*)(xtc->payload()));
    break;
        default:
    printf("Unsupported acqiris lAcqConfiguration version %d\n",version);
    break;
        }
      break;
    }
  case (TypeId::Id_AcqWaveform) :
    if (info.device()==Pds::DetInfo::Acqiris)
      process(info, *(Acqiris::DataDescV1*)(xtc->payload()));
    break;            
  case (TypeId::Id_Opal1kConfig) :
    {
      process(info, *(const Opal1k::ConfigV1*) xtc->payload() );
      break;
    }
  case (TypeId::Id_TM6740Config) :
    {
      switch(xtc->contains.version()) {
      case 1:
        process(info, *(const Pulnix::TM6740ConfigV1*) xtc->payload() );
        break;
      case 2:
        process(info, *(const Pulnix::TM6740ConfigV2*) xtc->payload() );
        break;
      default:
        break;
      }
      break;
    }
  case (TypeId::Id_FccdConfig) :
    {
      switch(xtc->contains.version()) {
      case 2:
        process(info, *(const FCCD::FccdConfigV2*) xtc->payload() );
        break;
      default:
        printf("Unsupported FCCD config version %d\n", xtc->contains.version());
        break;
      }
      break;
    }
  case (TypeId::Id_FrameFexConfig) :
    {
      process(info, *(const Camera::FrameFexConfigV1*) xtc->payload() );
      break;
    }
  case (TypeId::Id_TwoDGaussian) :
    {
      process(info, *(const Camera::TwoDGaussianV1*) xtc->payload() );
      break;
    }
  case (TypeId::Id_Frame) :
    {
      process(info, *(const Camera::FrameV1*) xtc->payload() );
      break;
    }
  case (TypeId::Id_Epics) :
    {
      process(info, *(const EpicsPvHeader*) xtc->payload() );
      break;            
    }
  case (TypeId::Id_pnCCDconfig) :
    {
      process(info, *(const PNCCD::ConfigV1*)(xtc->payload()));
      break;
    }
  case (TypeId::Id_pnCCDframe) :
    {
      process(info, (const PNCCD::FrameV1*)(xtc->payload()));
      break;
    }        
  case (TypeId::Id_PrincetonConfig) :
    {
      process(info, *(const Princeton::ConfigV1*)(xtc->payload()));
      break;
    }
  case (TypeId::Id_PrincetonFrame) :
    {
      process(info, (const Princeton::FrameV1*)(xtc->payload()));
      break;
    }        
    case (TypeId::Id_PrincetonInfo) :
      {
        process(info, (const Princeton::InfoV1*)(xtc->payload()));
        break;
      }        
  case (TypeId::Id_IpimbConfig) :
    {
      process(info, *(const Ipimb::ConfigV1*)(xtc->payload()));
      break;
    }
  case (TypeId::Id_IpimbData) :
    {
      process(info, (const Ipimb::DataV1*)(xtc->payload()));
      break;
    }        
  case (TypeId::Id_EncoderData) :
    {
      switch(xtc->contains.version()) {
      case 1 : process(info, (const Encoder::DataV1*) xtc->payload() ); break;
      case 2 : process(info, (const Encoder::DataV2*) xtc->payload() ); break;
      default: break;
      }          
      break;
    }
  case (TypeId::Id_EncoderConfig) :
    {
      process(info, *(const Encoder::ConfigV1*)(xtc->payload()));
      break;
    }        
  case (TypeId::Id_IpmFexConfig) :
    {
      process(info, (const Lusi::IpmFexConfigV1*)(xtc->payload()));
      break;
    }
  case (TypeId::Id_IpmFex) :
    {
      process(info, (const Lusi::IpmFexV1*)(xtc->payload()));
      break;
    }
  case (TypeId::Id_DiodeFexConfig) :
    {
      process(info, (const Lusi::DiodeFexConfigV1*)(xtc->payload()));
      break;
    }
  case (TypeId::Id_DiodeFex) :
    {
      process(info, (const Lusi::DiodeFexV1*)(xtc->payload()));
      break;
    }
  case (TypeId::Id_CspadConfig) :
    {
      switch( xtc->contains.version() ) {
      case 1: process(info, *(const CsPad::ConfigV1*)(xtc->payload())); break;
      case 2: process(info, *(const CsPad::ConfigV2*)(xtc->payload())); break;
      default:
	printf("Unknown version %d of Id_CspadConfig\n",xtc->contains.version());
	break;
      } break;
    }
  case (TypeId::Id_CspadElement) :
    lCspadData = xtc;
    break;
  case (TypeId::Id_ControlConfig) :
    {
      process(info, *(const ControlData::ConfigV1*)(xtc->payload()));
      break;        
    }
  case (TypeId::Id_EvrConfig) :
    {      
      unsigned version = xtc->contains.version();
      switch (version) {
      case 1:
        process(info, *(const EvrData::ConfigV1*)(xtc->payload()));
        break;
      case 2:
        process(info, *(const EvrData::ConfigV2*)(xtc->payload()));
        break;
      case 3:
        process(info, *(const EvrData::ConfigV3*)(xtc->payload()));
        break;
      case 4:
        process(info, *(const EvrData::ConfigV4*)(xtc->payload()));
        break;
      default:
        printf("Unsupported evr configuration version %d\n",version);
        break;
      }
      break;      
    }
  case (TypeId::Id_EvrData) :
    {
      process(info, *(const EvrData::DataV3*) xtc->payload() );
      break;        
    }    
  case (TypeId::Id_PimImageConfig) :
    break;
  default :
    // undamaged event with unrecognized data type
    printf("Type 0x%x not handled\n", xtc->contains.id());
    // break;
    // this traps an unusual source of xtc header corruption in AMO early days
    return Stop;
  }
    }        
    return Continue;
  }
private:
  unsigned int _depth;
  int _iDebugLevel;
};

void usage(char* progname) 
{
  fprintf(stderr,"Usage: %s -f <filename> -l <filewithfilelist> -s <skipevts> -n <maxevts> -d <debug level> [-h]\n", progname);
  fprintf(stderr,"Usage: The -l argument requires a file with a list of files in it.\n");
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

static void dump(Pds::Dgram* dg)
{
  char buff[128];
  time_t t = dg->seq.clock().seconds();
  strftime(buff,128,"%H:%M:%S",localtime(&t));
  printf("%s.%09u %08x/%08x %s extent 0x%x damage %x\n",
	 buff,
	 dg->seq.clock().nanoseconds(),
	 dg->seq.stamp().fiducials(),dg->seq.stamp().vector(),
	 Pds::TransitionId::name(dg->seq.service()),
	 dg->xtc.extent, dg->xtc.damage.value());
}

static void dump(Pds::Dgram* dg, unsigned ev)
{
  char buff[128];
  time_t t = dg->seq.clock().seconds();
  strftime(buff,128,"%H:%M:%S",localtime(&t));
  printf("%s.%09u %08x/%08x %s extent 0x%x damage %x event %d\n",
	 buff,
	 dg->seq.clock().nanoseconds(),
	 dg->seq.stamp().fiducials(),dg->seq.stamp().vector(),
	 Pds::TransitionId::name(dg->seq.service()),
	 dg->xtc.extent, dg->xtc.damage.value(), ev);
}

void anarun(XtcRun& run, unsigned &maxevt, unsigned &skip, int iDebugLevel)
{
  run.init();

  char* buffer = new char[0x2000000];
  Pds::Dgram* dg = (Pds::Dgram*)buffer;
  printf("Using buffer %p\n",buffer);

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

    if (dg->seq.service()!=Pds::TransitionId::L1Accept) 
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
    _fiducials = dg->seq.stamp().fiducials();
    damage = dg->xtc.damage.value();
    if (damage)
    {
      ndamage++;
      damagemask |= damage;
    }

    clockTimeCurDatagram = dg->seq.clock();
    myLevelIter iter(&(dg->xtc), 0, iDebugLevel);
    
    if (dg->seq.service() == TransitionId::L1Accept)
    {
      if (maxevt)
        maxevt--;
      else
        break;
      nevent++;

      resetEventData();

      iter.iterate();
      event();
    }
    else {
      if (dg->seq.service() == TransitionId::Configure)
  {
    iter.iterate();
    
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

    iter.iterate();
    beginrun();
  }    
      else if (dg->seq.service() == TransitionId::BeginCalibCycle)
  {
    iter.iterate();
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

//  printf("Opening output file %s\n", outfile);
//  TFile *out;
//  out = new TFile(outfile, "RECREATE");

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
    resetStaticData();
    printf("Analyzing files %s [%d]\n", 
     run.base(),nfiles);
    anarun(run, maxevt, skip, iDebugLevel);
    run.reset(*it);
    nfiles=0;
  }
  nfiles++;
      }
      resetStaticData();
      printf("Analyzing files %s [%d]\n", 
       run.base(),nfiles);
      anarun(run, maxevt, skip, iDebugLevel);
    }
    else
      printf("Unable to open list of files %s\n", filelist);
  }
  endjob();

//  out->Write();
//  out->Close();

  return 0;
}
