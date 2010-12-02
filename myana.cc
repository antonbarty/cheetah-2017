/* $Id: myana.cc,v 1.14 2010/07/22 22:26:57 caf Exp $ */
#include <TROOT.h>
#include <TH1F.h>
#include <TProfile.h>

#include "myana.hh"
#include "main.hh"
    
static int        numChannelsITof;
static int        numSamplesITof;
static double     sampleIntervalITof;
static TProfile*  profileITof = NULL;

// This function is called once at the beginning of the analysis job,
// You can ask for detector "configuration" information here.

void beginjob() 
{
  int fail = 0;
  /*
   * Processing ITof Config Data
   */     
  fail = getAcqConfig( AmoITof, numChannelsITof, numSamplesITof, sampleIntervalITof);
  if ( fail != 0 )
    printf( "begin(): getAcqConfig() failed, code = %d\n", fail );
  
  // create an "averaging" histogram (called "profile")
  profileITof = new TProfile("avg","avg",numSamplesITof,
                             0.0,sampleIntervalITof,"");
  profileITof->SetYTitle("Volts");    //optional
  profileITof->SetXTitle("Seconds");//optional
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
  int i;
  int fail = 0;
  /*
   * Processing ITof waveform
   */     
  double* timeITof;
  double* voltageITof;
  int channel = 0;
  fail = getAcqValue( AmoITof, channel, timeITof, voltageITof);
  if ( fail != 0 )
  {
    printf( "event(): getAcqValue() failed, code = %d\n", fail );
  }
  else
  {
    for (i=0;i<numSamplesITof;i=i+1) 
      {
        double t = timeITof[i];
        double v = voltageITof[i];
        profileITof->Fill(t,v);
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
