/* $Id: main.hh,v 1.38 2011/02/04 22:51:43 weaver Exp $ */
#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdio.h>
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/BldInfo.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
/*
class TH1;
*/
/*
 * Time functions
 */
int getTime( int& seconds, int& nanoSeconds );
int getLocalTime( const char*& time );
int getFiducials(unsigned& fiducials);
int getRunNumber();

/*
 * Enum definitions
 *
 * (copied from pdsdata/xtc/DetInfo.hh)
 */
enum AcqDetector
{
  AmoIms, AmoGasdet, AmoETof, AmoITof, AmoMbes, AmoVmiAcq, AmoBpsAcq, Camp,
  SxrBeamlineAcq1, SxrBeamlineAcq2, SxrEndstationAcq1, SxrEndstationAcq2
};

enum FrameDetector
{
  AmoVmi, AmoBps1, AmoBps2, 
  SxrBeamlineOpal1, SxrBeamlineOpal2, SxrEndstationOpal1, SxrEndstationOpal2,
  SxrFccd,
  XppSb1PimCvd, XppMonPimCvd, XppSb3PimCvd, XppSb4PimCvd,
  XppEndstationCam1
};

/*
 * Configuration retrieval functions
 */
int getAcqConfig      (AcqDetector det, int& numChannels, int& numSamples, double& sampleInterval);
int getAcqConfig      (Pds::DetInfo det, int& numChannels, int& numSamples, double& sampleInterval);
#define getOpal1kConfig(x)  getFrameConfig(x)
#define getTm6740Config(x)  getFrameConfig(x)
int getFrameConfig   (FrameDetector det);
int getFrameConfig   (Pds::DetInfo  det);
int getPrincetonConfig(Pds::DetInfo::Detector det, int iDevId, int& width, int& height, int& orgX, int& orgY, int& binX, int&binY);
int getIpimbConfig   (Pds::DetInfo::Detector det, int iDevId, uint64_t& serialID,
                      int& chargeAmpRange0, int& chargeAmpRange1,
                      int& chargeAmpRange2, int& chargeAmpRange3);
int getEncoderConfig   (Pds::DetInfo::Detector det, int iDevId);
int getFccdConfig(FrameDetector det, uint16_t& outputMode, bool& ccdEnable, bool& focusMode, uint32_t& exposureTime,
                  float& dacVoltage1, float& dacVoltage2, float& dacVoltage3, float& dacVoltage4,
                  float& dacVoltage5, float& dacVoltage6, float& dacVoltage7, float& dacVoltage8,
                  float& dacVoltage9, float& dacVoltage10, float& dacVoltage11, float& dacVoltage12,
                  float& dacVoltage13, float& dacVoltage14, float& dacVoltage15, float& dacVoltage16,
                  float& dacVoltage17,
                  uint16_t& waveform0, uint16_t& waveform1, uint16_t& waveform2, uint16_t& waveform3,
                  uint16_t& waveform4, uint16_t& waveform5, uint16_t& waveform6, uint16_t& waveform7,
                  uint16_t& waveform8, uint16_t& waveform9, uint16_t& waveform10, uint16_t& waveform11,
                  uint16_t& waveform12, uint16_t& waveform13, uint16_t& waveform14);
int getDiodeFexConfig (Pds::DetInfo::Detector det, int iDevId, float* base, float* scale);
int getIpmFexConfig   (Pds::DetInfo::Detector det, int iDevId, 
           float* base0, float* scale0,
           float* base1, float* scale1,
           float* base2, float* scale2,
           float* base3, float* scale3,
           float& xscale, float& yscale);

namespace Pds { namespace CsPad { class ConfigV1; class ConfigV2; class ConfigV3; }}
int getCspadConfig (Pds::DetInfo::Detector det, Pds::CsPad::ConfigV1& cfg);
int getCspadConfig (Pds::DetInfo::Detector det, Pds::CsPad::ConfigV2& cfg);
int getCspadConfig (Pds::DetInfo::Detector det, Pds::CsPad::ConfigV3& cfg);

/* Note: Shared BLD Ipimb Config available on L1Accept Data Event */
int getBldIpimbConfig(Pds::BldInfo::Type bldType, uint64_t& serialID, int& chargeAmpRange0,
                      int& chargeAmpRange1, int& chargeAmpRange2, int& chargeAmpRange3);


/*
 * L1Accept Data retrieval functions
 */
int getAcqValue   (AcqDetector det, int channel, double*& time, double*& voltage);
int getAcqValue   (AcqDetector det, int channel, double*& time, double*& voltage, double& trigtime);
int getAcqValue   (Pds::DetInfo det, int channel, double*& time, double*& voltage, double& trigtime);
#define getOpal1kValue(w, x, y, z) getFrameValue(w, x, y, z)
#define getTm6740Value(w, x, y, z) getFrameValue(w, x, y, z)
int getFrameValue(FrameDetector det, int& frameWidth, int& frameHeight, unsigned short*& image );
int getFrameValue(Pds::DetInfo  det, int& frameWidth, int& frameHeight, unsigned short*& image );
int getPrincetonValue      (Pds::DetInfo::Detector det, int iDevId, unsigned short *& image);
int getPrincetonTemperature(Pds::DetInfo::Detector det, int iDevId, float& temperature);
int getIpimbVolts          (Pds::DetInfo::Detector det, int iDevId, 
                            float &channel0, float &channel1, float &channel2, float &channel3);
int getFeeGasDet  (double* shotEnergy);
int getEBeam      (double& charge, double& energy, double& posx, double& posy,
                   double& angx, double& angy);
int getEBeam      (double& charge, double& energy, double& posx, double& posy,
                   double& angx, double& angy, double& pkcurr);
int getPhaseCavity(double& fitTime1, double& fitTime2, double& charge1,  double& charge2);

int getPvInt      (const char* pvName, int& value);
int getPvFloat    (const char* pvName, float& value);
int getPvString   (const char* pvName, char*& value);
int getPnCcdConfig(int deviceId, const Pds::PNCCD::ConfigV1*& c );
int getPnCcdRaw   (int deviceId, const Pds::PNCCD::FrameV1*& frame );
int getPnCcdValue (int deviceId, unsigned char*& image, int& width, int& height );
int getEvrDataNumber();
int getEvrData    (int id, unsigned int& eventCode, unsigned int& fiducial, unsigned int& timeStamp);
int getEncoderCount(Pds::DetInfo::Detector det, int iDevId, int& encoderCount, int chan=0);
int getDiodeFexValue (Pds::DetInfo::Detector det, int iDevId, float& value);
int getIpmFexValue   (Pds::DetInfo::Detector det, int iDevId, 
                      float* channels, float& sum, float& xpos, float& ypos);

namespace Pds { namespace CsPad { class ElementIterator; }}
int getCspadData  (Pds::DetInfo::Detector det, Pds::CsPad::ElementIterator& iter);
int getBldIpimbVolts(Pds::BldInfo::Type bldType, float &channel0, float &channel1,
                     float &channel2, float &channel3);
int getBldIpmFexValue(Pds::BldInfo::Type bldType, float* channels, 
                      float& sum, float& xpos, float& ypos);

/*
 * Control data retrieval functions
 */
int getControlPvNumber();
int getControlPvName  (int pvId, const char*& pvName, int& arrayIndex); 
int getControlValue   (const char* pvName, int arrayIndex, double& value);

int getMonitorPvNumber();
int getMonitorPvName  (int pvId, const char*& pvName, int& arrayIndex); 
int getMonitorValue   (const char* pvName, int arrayIndex, double& hilimit, double& lolimit);

/* 
 * Advanced Interface
 */
int   getEpicsPvNumber();
int   getEpicsPvConfig(int iPvId, const char*& sPvName, int& iType, int& iNumElements);
int   getEpicsPvValue (int pvId, const void*& value, int& dbrype, struct tm& tmTimeStamp, int& nanoSec);
/*
void  fillConstFrac(double* t, double* v, unsigned numSamples, float baseline,
                   float thresh, TH1* hist);
*/
void  fillConstFrac(double* t, double* v, unsigned numSamples, float baseline,
        float thresh, double* edge, int& n, int maxhits);

class XtcRun;
XtcRun* getDarkFrameRun(unsigned run);

#endif

