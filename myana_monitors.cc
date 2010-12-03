/* $Id: myana_morefeatures.cc,v 1.28 2010/10/14 19:36:55 jana Exp $ */
#include <TROOT.h>
#include <TH1F.h>
#include <TProfile.h>
#include <math.h>

#include "myana.hh"
#include "main.hh"

#include "release/pdsdata/cspad/ElementIterator.hh"
    

static int                  CCDwidth;
static int                  CCDheight;
static int                  orgXSxrRCI0;
static int                  orgYSxrRCI0;
static int                  binXSxrRCI0;
static int                  binYSxrRCI0;
unsigned short*             princetonImage;

static const char*          pvNames[] = { "AMO:DIA:SHC:11:R" };
static const int            numPv = sizeof(pvNames) / sizeof(pvNames[0]);

static int                  numControlPv = 0;
static int                  numMonitorPv = 0;
unsigned int                event_i;
unsigned int                calib_i;
bool                        ccd_failed;

using namespace Pds;

// This function is called once at the beginning of the analysis job,
// You can ask for detector "configuration" information here.

void beginjob() {
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

    //printf( "Control PV %d: %s [%d] = %lf\n", controlPv, pvName, arrayIndex, value );
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

    //printf( "Monitor PV %d: %s [%d] ", monitorPv, pvName, arrayIndex );
    printf( "High Limit %lf  Low Limit %lf\n", hilimit, lolimit );
  }  
  
  fail = getPrincetonConfig( DetInfo::XppEndstation, 0, CCDwidth, CCDheight,
    orgXSxrRCI0, orgYSxrRCI0, binXSxrRCI0, binYSxrRCI0 );
  if ( fail == 0 )
  {
    //printf( "Get Princeton config for SxrRCI0: width %d height %d\n",
    //  CCDwidth, CCDheight);
  }
  // Generate column header.
  printf("#time\tphaseCavityTime1[fs]\tphaseCavityTime2[fs]\tebenergy[MeV]\n");

}

// This function is called once for each run.  You should check to see
// if detector configuration information has changed.

void beginrun() 
{
  calib_i=0;
}

void begincalib()
{
  event_i=0;
  calib_i++;
  //printf("Begin calib cycle %d\n",calib_i);
}

// This is called once every shot.  You can ask for
// the individual detector shot data here.

void event() 
{
  if (event_i==0) {
    event_i++;
    ccd_failed=getPrincetonValue( DetInfo::XppEndstation, 0, princetonImage);
    return;
  }
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
   * Processing FeeGasDet Value
   */     
  double shotEnergy[4];
  fail = getFeeGasDet( shotEnergy );
    
  /*
   * Get data from EBeam (i.e. accelerator)
   */     
  double ebcharge=nan(""), ebenergy=nan(""), posx, posy, angx, angy;
  fail =  getEBeam(ebcharge, ebenergy, posx, posy, angx, angy);

  /*
   * Get phase cavity data
   */     
  double phaseCavityTime1=nan(""), phaseCavityTime2=nan(""), phaseCavityCharge1,
    phaseCavityCharge2;
  fail = getPhaseCavity( phaseCavityTime1, phaseCavityTime2,
    phaseCavityCharge1, phaseCavityCharge2 );

/*
  // loop over a list of PVs
  for ( int iPv = 0; iPv < numPv; iPv++ )
    {
      fail = getPvFloat( pvNames[iPv], value );
      if ( fail != 0 ) // getPvXXX() failed. Possibly because that pv is not recorded in current datagram
        continue;  
      printf( "Pv %s = %f\n", pvNames[iPv], value );
    }
*/    
 
//  float temperature;
//  fail = getPrincetonTemperature( DetInfo::XppEndstation, 0, temperature);
/*  if ( fail == 0 )
  {
    // Uncomment the following line to print out the princeton temperature 
    printf( "Princeton at XPPBeamline : temperature = %f\n", temperature );
  }    
*/
  
  float diodes[4];
  float ipm2sum,ipm3sum, xpos, ypos;
  fail = getIpmFexValue(DetInfo::XppSb2Ipm, 0, diodes, ipm2sum, xpos, ypos);
  fail = getIpmFexValue(DetInfo::XppSb3Ipm, 0, diodes, ipm3sum, xpos, ypos);
  if ( fail == 0 )
  {
    //printf( "IPM2,IPM3 (sum): %g,%g\n", ipm2sum,ipm3sum);
  }
  signed long tot;
  //unsigned ADU_per_ph=800;
  if (ccd_failed==0) {
    for (i=0;i<CCDheight*CCDwidth;i++) {
      tot+=princetonImage[i]-126;
    }
  }

  if (!(isnan(phaseCavityTime1) && isnan(phaseCavityTime2) && isnan(ebenergy)))
    printf("%s\t%g\t%g\t%g\n", time, phaseCavityTime1, phaseCavityTime2,
      ebenergy);
  event_i++;
}

void endcalib()
{
  event_i=0;
  //printf("User endcalib\n");
}

void endrun() 
{
  //printf("User analysis endrun() routine called.\n");
}

void endjob()
{
  //printf("User analysis endjob() routine called.\n");
}
