/* $Id: myana_pnccd.cc,v 1.2 2010/12/04 01:53:28 caf Exp $ */
#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>

#include "../myana.hh"
#include "../main.hh"
#include "../release/pdsdata/pnCCD/ConfigV1.hh"
#include "../release/pdsdata/pnCCD/ConfigV2.hh"
#include "PnccdCorrector.hh"

using namespace std;

static PnccdCorrector*      corrector;

static Pds::PNCCD::ConfigV1 configV1;
static Pds::PNCCD::ConfigV2 configV2;
static unsigned configVsn = 0;

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

  corrector = new PnccdCorrector(Pds::DetInfo::Camp,0,
                                 PnccdCorrector::DarkFrameOffset);
  printf("beginjob\n");
}

// This function is called once for each run.  You should check to see
// if detector configuration information has changed.

void fetchConfig()
{
  configVsn= 0;
}

void beginrun() 
{
  fetchConfig();
  printf("beginrun\n");

  if (corrector) {
    corrector->loadConstants(getRunNumber());
  }
}

void begincalib()
{
  fetchConfig();
  printf("begincalib\n");
}

// This is called once every shot.  You can ask for
// the individual detector shot data here.

void event() 
{
  static int ievent = 0;

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

  //
  // get pnCCD frame and apply correction
  //

  int pnccdWidth, pnccdHeight;
  unsigned char *pnccdImage = (unsigned char *)NULL;
  float *corrected = (float *)NULL;

  fail = getPnCcdValue (0, pnccdImage, pnccdWidth, pnccdHeight );
  if (fail) {
    printf("getPnCcdValue() failed\n");
  } else if (corrector && pnccdImage) {
    corrected = corrector->apply(pnccdImage);
  }

  //
  //  Dump some data from the first events
  //
  if (ievent < 2) {
    uint16_t *raw = (uint16_t *)pnccdImage;

    printf("=== event #%d ==============================================\n",
            ievent+1);
    printf("  raw pixels:       %-10d %-10d %-10d ...\n",
            raw[0], raw[1], raw[2]);
    printf("  corrected pixels: %f %f %f ...\n",
            corrected[0], corrected[1], corrected[2]);
    printf("===========================================================\n");
  }


  //
  //  Do some real analysis here
  //

  ++ievent;
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
