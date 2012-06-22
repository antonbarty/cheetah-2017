/* $Id: myana.cc,v 1.16 2012/02/16 21:55:20 caf Exp $ */
#include <TROOT.h>
#include <TH1F.h>
#include <TProfile.h>

#include "myana.hh"
#include "main.hh"
    
// This function is called once at the beginning of the analysis job,
// You can ask for detector "configuration" information here.

void beginjob() 
{
  int fail = 0;

  fail = getFrameConfig(MecEndstationSid4);
  if ( fail != 0 )
    printf( "begin(): getFrameConfig(MecEndstationSid4) failed, code = %d\n", fail );
}

// This function is called once for each run.  You should check to see
// if detector configuration information has changed.

void beginrun() 
{
  int fail = 0;
  Pds::CsPad2x2::ConfigV1 cfgV1;
  Pds::CsPad::ConfigV3 cfgV3;

  fail = getCspad2x2Config (Pds::DetInfo::XcsEndstation, cfgV1);
  if (fail) {
    printf("beginrun: getCspad2x2Config (Pds::DetInfo::XcsEndstation, cfgV1) failed\n");
  }
  fail = getCspad2x2Config (Pds::DetInfo::XcsEndstation, cfgV3);
  if (fail) {
    printf("beginrun: getCspad2x2Config (Pds::DetInfo::XcsEndstation, cfgV3) failed\n");
  }
}

void begincalib()
{
}

// This is called once every shot.  You can ask for
// the individual detector shot data here.

void event() 
{
  int fail = 0;
  int width, height;
  uint16_t *image;
  static uint16_t frameCounter = 1;

  fail = getFrameValue(MecEndstationSid4, width, height, image);
  if ( fail != 0 ) {
    printf( "event(): getFrameValue(MecEndstationSid4) failed, code = %d\n", fail );
  } else {
    printf("MecEndstationSid4: W x H = %d x %d\n", width, height);
  }
  ++ frameCounter;
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
