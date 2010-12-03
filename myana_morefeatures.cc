/* $Id: myana_morefeatures.cc,v 1.28 2010/10/14 19:36:55 jana Exp $ */
#include <TROOT.h>
#include <TH1F.h>
#include <TProfile.h>

#include "myana.hh"
#include "main.hh"

#include "release/pdsdata/cspad/ElementIterator.hh"
    
static const int            numITofShot = 5;
static const int            maxETofChannel = 20;

static int                  numChannelsITof;
static int                  numSamplesITof;
static double               sampleIntervalITof;
static TProfile*            profileITof = NULL;
static TH1F*                h1fITof[numITofShot];
static TH1F*                constFracITof;
static unsigned int         shotCountITof = 0;

static int                  numChannelsETof;
static int                  numSamplesETof;
static double               sampleIntervalETof;
static TProfile*            profileETof[maxETofChannel];

static int                  numChannelsMbes;
static int                  numSamplesMbes;
static double               sampleIntervalMbes;

static int                  widthSxrRCI0;
static int                  heightSxrRCI0;
static int                  orgXSxrRCI0;
static int                  orgYSxrRCI0;
static int                  binXSxrRCI0;
static int                  binYSxrRCI0;

static uint16_t             outputMode;
static bool                 ccdEnable;
static bool                 focusMode;
static uint32_t             exposureTime;
static float                dacVoltage1, dacVoltage2, dacVoltage3, dacVoltage4;
static float                dacVoltage5, dacVoltage6, dacVoltage7, dacVoltage8;
static float                dacVoltage9, dacVoltage10, dacVoltage11, dacVoltage12;
static float                dacVoltage13, dacVoltage14, dacVoltage15, dacVoltage16;
static float                dacVoltage17;
static uint16_t             waveform0, waveform1, waveform2, waveform3;
static uint16_t             waveform4, waveform5, waveform6, waveform7;
static uint16_t             waveform8, waveform9, waveform10, waveform11;
static uint16_t             waveform12, waveform13, waveform14;

static const char*          pvNames[] = { "AMO:DIA:SHC:11:R" };
static const int            numPv = sizeof(pvNames) / sizeof(pvNames[0]);

static int                  numControlPv = 0;
static int                  numMonitorPv = 0;

using namespace Pds;

// This function is called once at the beginning of the analysis job,
// You can ask for detector "configuration" information here.

void beginjob() {
  int i;
  int fail = 0;
  /*
   * Get time information
   */
  int seconds, nanoSeconds;
  getTime( seconds, nanoSeconds );
    
  const char* time;
  fail = getLocalTime( time );
  
  /*
   * Processing control (calibration) data
   */
  numControlPv = getControlPvNumber();
  for (int controlPv = 0; controlPv < numControlPv; controlPv++ )
  {
    const char* pvName;
    int arrayIndex;
    
    fail = getControlPvName( controlPv, pvName, arrayIndex );
    if ( fail != 0 ) continue;    
    
    double value;
    fail = getControlValue(pvName, arrayIndex, value);
    if ( fail != 0 ) continue;

    printf( "Control PV %d: %s [%d] = %lf\n", controlPv, pvName, arrayIndex, value );
  }  

  numMonitorPv = getMonitorPvNumber();
  for (int monitorPv = 0; monitorPv < numMonitorPv; monitorPv++ )
  {
    const char* pvName;
    int arrayIndex;
    
    fail = getMonitorPvName( monitorPv, pvName, arrayIndex );
    if ( fail != 0 ) continue;    
    
    double hilimit, lolimit;
    fail = getMonitorValue(pvName, arrayIndex, hilimit, lolimit );    
    if ( fail != 0 ) continue;

    printf( "Monitor PV %d: %s [%d] ", monitorPv, pvName, arrayIndex );
    printf( "High Limit %lf  Low Limit %lf\n", hilimit, lolimit );
  }  
  
  /*
   * Processing ITof Config Data
   */     
  fail = getAcqConfig( AmoITof, numChannelsITof, numSamplesITof, sampleIntervalITof);
    
  // create an "averaging" histogram (called "profile")
  profileITof = new TProfile("avg","avg",numSamplesITof,
                             0.0,sampleIntervalITof,"");
  profileITof->SetYTitle("Volts");    //optional
  profileITof->SetXTitle("Seconds");//optional

  // create a constant-fraction histogram for itof.
  constFracITof = new TH1F("ITOF Constant Fraction","ITOF Constant Fraction",
                           numSamplesITof,0.0,sampleIntervalITof);
        
  char name[32];
  // create 5 individual shot histograms
  for (i=0;i<numITofShot;i=i+1) 
    {
      sprintf(name,"shot%3d",i);
      h1fITof[i] = new TH1F(name,name,numSamplesITof,0.0,sampleIntervalITof);
      h1fITof[i]->SetYTitle("Volts");    //optional
      h1fITof[i]->SetXTitle("Seconds");//optional
    }    

  /*
   * Processing ETof Config Data.  Make an averaging histogram for each channel.
   */
  fail = getAcqConfig( AmoETof, numChannelsETof, numSamplesETof, sampleIntervalETof);    
  // create 5 individual shot histograms
  for (i=0;i<numChannelsETof;i=i+1) {
    sprintf(name,"ETOF Channel %d",i);
    profileETof[i] = new TProfile(name,name,numSamplesETof,
                                  0.0,sampleIntervalETof,"");
    profileETof[i]->SetYTitle("Volts");    //optional
    profileETof[i]->SetXTitle("Seconds");//optional
  }    
    
  /*
   * Processing Mbes Config Data
   */
  fail = getAcqConfig( AmoMbes, numChannelsMbes, numSamplesMbes, sampleIntervalMbes);                  
  
  
  /*
   * Processing Princeton Config Data
   */
  fail = getPrincetonConfig( DetInfo::SxrBeamline, 0, widthSxrRCI0, heightSxrRCI0, orgXSxrRCI0, orgYSxrRCI0, binXSxrRCI0, binYSxrRCI0 );
  if ( fail == 0 )
  {
    printf( "Get Princeton config for SxrRCI0: width %d height %d  org (%d,%d)  binX %d binY %d\n",
      widthSxrRCI0, heightSxrRCI0, orgXSxrRCI0, orgYSxrRCI0, binXSxrRCI0, binYSxrRCI0 );
  }

  /*
   * Processing FCCD Config Data
   */

  fail = getFccdConfig(SxrFccd, outputMode, ccdEnable, focusMode, exposureTime,
                  dacVoltage1, dacVoltage2, dacVoltage3, dacVoltage4,
                  dacVoltage5, dacVoltage6, dacVoltage7, dacVoltage8,
                  dacVoltage9, dacVoltage10, dacVoltage11, dacVoltage12,
                  dacVoltage13, dacVoltage14, dacVoltage15, dacVoltage16,
                  dacVoltage17,
                  waveform0, waveform1, waveform2, waveform3,
                  waveform4, waveform5, waveform6, waveform7,
                  waveform8, waveform9, waveform10, waveform11,
                  waveform12, waveform13, waveform14);
}

// This function is called once for each run.  You should check to see
// if detector configuration information has changed.

void beginrun() 
{
  int fail = 0;
  int     numChannelsITof2 = 0;
  int     numSamplesITof2 = 0;
  double  sampleIntervalITof2 = 0;
  
  // Example: AMO ITOF
  fail = getAcqConfig( AmoITof, numChannelsITof2, numSamplesITof2, sampleIntervalITof2);
  if ( fail != 0 ) {
    printf( "beginrun(): getAcqConfig() failed, code = %d\n", fail );
  } else if ( numChannelsITof2 != numChannelsITof || numSamplesITof2 != numSamplesITof ||
              sampleIntervalITof2 != sampleIntervalITof ) {
    printf( "The ITof configuration has been changed between runs!\n" );
  }
}

void begincalib()
{
}

// This is called once every shot.  You can ask for
// the individual detector shot data here.

void event() 
{
  int i,j;
  int fail = 0;
  /*
   * Get time information
   */
  int seconds, nanoSeconds;
  getTime( seconds, nanoSeconds );
    
  const char* time;
  fail = getLocalTime( time );

  unsigned fiducials;
  fail = getFiducials(fiducials);

  /*
   * Processing EVR data
   */     
  int numEvrData = getEvrDataNumber();
  for (i=0; i< numEvrData; i++)
  {
    unsigned int eventCode;
    unsigned int fiducial;
    unsigned int timeStamp;
    
    fail = getEvrData( i, eventCode, fiducial, timeStamp );
    if ( fail != 0 )
    {
      printf( "event(): getEvrData(%d) failed, code = %d\n", i, fail );
      continue;
    }
    
    //// Uncomment the following lines to print out the evr data
    //printf( "EvrData [%d]: Event Code %u  Fiducial Id = 0x%x  TimeStamp = %u\n", 
    //  i, eventCode, fiducial, timeStamp );
  }
  
  /*
   * Processing ITof waveform
   */     
  double* timeITof;
  double* voltageITof;
  int channel = 0;
  fail = getAcqValue( AmoITof, channel, timeITof, voltageITof);
  if ( fail != 0 )
    printf( "event(): getAcqValue() failed, code = %d\n", fail );
  else
  {        
    float baseline  = -0.03;
    float threshold = -0.06;
    fillConstFrac(timeITof,voltageITof,numSamplesITof,
                  baseline,threshold,constFracITof);

    for (i=0;i<numSamplesITof;i=i+1) 
      {
        double t = timeITof[i];
        double v = voltageITof[i];
        profileITof->Fill(t,v);
        if (shotCountITof<5) h1fITof[shotCountITof]->Fill(t,v);
      }
    shotCountITof++;
  }

  /*
   * Processing ETof waveform
   */     
  double* timeETof;
  double* voltageETof;
  for (i=0;i<numChannelsETof;i=i+1) 
  {
    fail = getAcqValue( AmoETof, i, timeETof, voltageETof);
    if ( fail == 0 )
    {
      for (j=0;j<numSamplesETof;j=j+1) 
        {
          double t = timeETof[j];
          double v = voltageETof[j];
          profileETof[i]->Fill(t,v);
        }
    }
  }
    
  /*
   * Processing Mbes waveform
   */     
  double* timeMbes;
  double* voltageMbes;
  fail = getAcqValue( AmoMbes, channel, timeMbes, voltageMbes);    
    
  /*
   * Processing Vmi image
   */     
  int frameWidthVmi, frameHeightVmi;
  unsigned short* imageVmi;    
  fail = getOpal1kValue(AmoVmi, frameWidthVmi, frameHeightVmi, imageVmi );    

  /*
   * Processing Bps1 image
   */     
  int frameWidthBps1, frameHeightBps1;
  unsigned short* imageBps1;    
  fail = getOpal1kValue(AmoBps1, frameWidthBps1, frameHeightBps1, imageBps1 );    

  /*
   * Processing Bps2 image
   */     
  int frameWidthBps2, frameHeightBps2;
  unsigned short* imageBps2;    
  fail = getOpal1kValue(AmoBps2, frameWidthBps2, frameHeightBps2, imageBps2 );        
    
  /*
   * Processing FeeGasDet Value
   */     
  double shotEnergy[4];
  fail = getFeeGasDet( shotEnergy );
    
  /*
   * Get data from EBeam (i.e. accelerator)
   */     
  double ebcharge; double ebenergy; double posx; double posy;
  double angx; double angy;
  fail =  getEBeam(ebcharge, ebenergy, posx, posy, angx, angy);

  /*
   * Get phase cavity data
   */     
  double phaseCavityTime1, phaseCavityTime2, phaseCavityCharge1, phaseCavityCharge2;
  fail = getPhaseCavity( phaseCavityTime1, phaseCavityTime2, phaseCavityCharge1, phaseCavityCharge2 );

  /*
   * Get IPIMB data for DetInfo::SxrBeamline device ID 0
   */
  float volts0, volts1, volts2, volts3;
  fail = getIpimbVolts(DetInfo::SxrBeamline, 0, volts0, volts1, volts2, volts3);

  /*
   * Get encoder data for DetInfo::SxrBeamline device ID 0, channel 0 (default)
   */
  int chan, count;
  fail = getEncoderCount(DetInfo::SxrBeamline, 0, count, chan=0);

  /*
   * Processing Epics Pv Values
   */  
  // get one PV
  float value;
  fail = getPvFloat( "AMO:DIA:SHC:11:R", value );

  // loop over a list of PVs
  for ( int iPv = 0; iPv < numPv; iPv++ )
    {
      fail = getPvFloat( pvNames[iPv], value );
      if ( fail != 0 ) // getPvXXX() failed. Possibly because that pv is not recorded in current datagram
        continue;  
      printf( "Pv %s = %f\n", pvNames[iPv], value );
    }
    
  /*
   * Processing pnCCD image from device 0 
   */     
  unsigned char*  pnCcdImage0;
  int             pnCcdImageWidth0, pnCcdImageHeight0;
  fail = getPnCcdValue( 0, pnCcdImage0, pnCcdImageWidth0, pnCcdImageHeight0);
  if ( fail == 0 )
  {
    printf( "Get PnCCD Image from device %d, width = %d height = %d\n", 0, pnCcdImageWidth0, pnCcdImageHeight0 );
    //FILE* fPnCcd = fopen( "test0.raw", "wb" );
    //fwrite( pnCcdImage0, pnCcdImageWidth0 * pnCcdImageHeight0 * 2, 1, fPnCcd );
    //fclose( fPnCcd );
  }

  /*
   * Processing pnCCD image from device 1
   */     
  unsigned char*  pnCcdImage1;
  int             pnCcdImageWidth1, pnCcdImageHeight1;
  fail = getPnCcdValue( 1, pnCcdImage1, pnCcdImageWidth1, pnCcdImageHeight1);
  if ( fail == 0 )
  {
    printf( "Get PnCCD Image from device %d, width = %d height = %d\n", 1, pnCcdImageWidth1, pnCcdImageHeight1 );
  }  
  
  unsigned short*  princetonImageSxrRCI0;
  fail = getPrincetonValue( DetInfo::SxrBeamline, 0, princetonImageSxrRCI0);
  if ( fail == 0 )
  {
    printf( "Get Princeton Image from ScrBeamline 0\n" );
  }    
  float temperature;
  fail = getPrincetonTemperature( DetInfo::SxrBeamline, 0, temperature);
  if ( fail == 0 )
  {
    //// Uncomment the following line to print out the princeton temperature 
    //printf( "Princeton at ScrBeamline 0 : temperature = %f\n", temperature );
  }    

  
  /*
   * Processing TM6740 image from XPP
   */
  
  unsigned short* xppSb3Image;
  int xppSb3Width, xppSb3Height;
  fail = getTm6740Value(XppSb3PimCvd, xppSb3Width, xppSb3Height, xppSb3Image);
  if ( fail == 0 )
  {
    printf( "Get XppSb3Pim Image %d x %d\n", xppSb3Width, xppSb3Height);
  }

  fail = getIpimbVolts(DetInfo::XppSb3Ipm, 0, volts0, volts1, volts2, volts3);
  if ( fail == 0 )
  {
    printf( "Get XppSb3Ipm Diodes %g, %g, %g, %g\n", volts0, volts1, volts2, volts3);
  }

  fail = getDiodeFexValue(DetInfo::XppSb3Pim, 0, volts0);
  if ( fail == 0 )
  {
    printf( "Get XppSb3Pim Diode FEX %g\n", volts0);
  }

  float diodes[4];
  float sum, xpos, ypos;
  fail = getIpmFexValue(DetInfo::XppSb3Ipm, 0, diodes, sum, xpos, ypos);
  if ( fail == 0 )
  {
    printf( "Get XppSb3Ipm Diode FEX (%g, %g, %g, %g), %g, %g, %g\n", 
	    diodes[0], diodes[1], diodes[2], diodes[3],
	    sum, xpos, ypos);
  }

  Pds::CsPad::ElementIterator iter;
  fail = getCspadData(DetInfo::XppGon, iter);
  if ( fail == 0 ) {
    if ( (iter.next()) ) {
      unsigned section_id;
      const Pds::CsPad::Section* s = iter.next(section_id);
      if (s)
	printf("Get XppGon Pixels { %04x %04x %04x %04x }\n",
	       s->pixel[0][0], s->pixel[0][1], s->pixel[0][2], s->pixel[0][3]);
    }
  }
}

void endcalib()
{
}

void endrun() 
{
  printf("User analysis endrun() routine called.\n");
}

void endjob()
{
  printf("User analysis endjob() routine called.\n");
}
