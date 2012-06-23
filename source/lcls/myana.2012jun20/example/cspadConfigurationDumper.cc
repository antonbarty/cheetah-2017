/* $Id: cspadConfigurationDumper.cc,v 1.1 2010/11/13 18:13:24 philiph Exp $ */
#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>
#include <fstream>

#include "../myana.hh"
#include "../main.hh"
#include "../release/pdsdata/cspad/ConfigV1.hh"
#include "../release/pdsdata/cspad/ConfigV2.hh"
#include "../release/pdsdata/cspad/ElementHeader.hh"
#include "../release/pdsdata/cspad/ElementIterator.hh"

using namespace std;

static Pds::CsPad::ConfigV1 configV1;
static Pds::CsPad::ConfigV2 configV2;
static unsigned             configVsn;
static ofstream cspConfig;

static bool lowGain = false;
static const unsigned  ROWS = 194*2;
static const unsigned  COLS = 185;
static const unsigned TWOXONES = 8;
static const unsigned QUADS = 4;
static char* dumpFile = NULL;


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

  dumpFile = getenv("dumpConfigurationFile");
  if (!dumpFile) {
    dumpFile = "cspadConfigDump.txt";
    printf("did not find env variable dumpConfigurationFile, writing to default %s\n", dumpFile);
  } else {
    printf("Found env variable dumpPedestalFile with value %s\n", dumpFile);
  }
  cspConfig.open(dumpFile, ios::out);
  if (not cspConfig.is_open()) {
    printf("unable to open %s, bailing\n", dumpFile);
    exit(1);
  }
  printf("beginjob\n");
}

void dumpConfigV1(Pds::CsPad::ConfigV1 cfg) {
  cspConfig << "Cspad config V1 dump " << endl;//for detector " << (int) det << endl;
  cspConfig << "Version: " << CsPad::ConfigV1::Version << endl;
  cspConfig << "Tdi: 0x" << hex << cfg.tdi() << endl;
  cspConfig << "Quad mask: 0x" << hex << cfg.quadMask() << endl;
  cspConfig << "Run delay: 0x" << hex << cfg.runDelay() << endl;
  cspConfig << "Event code: 0x" << hex << cfg.eventCode() << endl;
  cspConfig << "Inactive run mode: 0x" << hex << cfg.inactiveRunMode() << endl;
  cspConfig << "Active run mode: 0x" << hex << cfg.activeRunMode() << endl;
  cspConfig << "Payload size: 0x" << hex << cfg.payloadSize() << endl;
  cspConfig << "Bad asic mask 0: 0x" << hex << cfg.badAsicMask0() << endl;
  cspConfig << "Bad asic mask 1: 0x" << hex << cfg.badAsicMask1() << endl;
  cspConfig << "Asic mask: 0x" << hex << cfg.asicMask() << endl;
  cspConfig << "Number of asics read: " << cfg.numAsicsRead() << endl;
  cspConfig << "Concentrator version: 0x" << cfg.concentratorVersion() << endl;
  for (int j=0; j<4; j++) {
    cspConfig << "Cspad quad " << j << " configuration:" << endl;
    for (int i=0; i<4; i++) {
      cspConfig << "     Shift select " << i << ": 0x" << cfg.quads()[j].shiftSelect()[i]<< endl;
      cspConfig << "     Edge select " << i << ": 0x" << cfg.quads()[j].edgeSelect()[i]<< endl;
    }
    cspConfig << "   Read clock set: 0x" << cfg.quads()[j].readClkSet()<< endl;
    cspConfig << "   Read clock hold: 0x" << cfg.quads()[j].readClkHold()<< endl;
    cspConfig << "   Data mode: 0x" << cfg.quads()[j].dataMode()<< endl;
    cspConfig << "   Prst sel: 0x" << cfg.quads()[j].prstSel()<< endl;
    cspConfig << "   Acq delay: 0x" << cfg.quads()[j].acqDelay()<< endl;
    cspConfig << "   Integration time: 0x" << cfg.quads()[j].intTime()<< endl;
    cspConfig << "   Digital delay: 0x" << cfg.quads()[j].digDelay()<< endl;
    cspConfig << "   Amp idle: 0x" << cfg.quads()[j].ampIdle()<< endl;
    cspConfig << "   Injection total: 0x" << cfg.quads()[j].injTotal()<< endl;
    cspConfig << "   Row col shift per: 0x" << cfg.quads()[j].rowColShiftPer()<< endl;
    cspConfig << "   ReadOnly shift test: 0x" << hex << cfg.quads()[j].ro().shiftTest<< endl;
    cspConfig << "   ReadOnly version: 0x" << hex << cfg.quads()[j].ro().version<< endl;
    for (unsigned i=0; i<CsPad::PotsPerQuad; i++) {
      cspConfig << "   Pot 0x" << i << " value: 0x" << (int)(cfg.quads()[j].dp().value(i))<< endl;
    }
    cspConfig << "Gain map for sample pixel 17/17   : 0x" << (*(cfg.quads()[j].gm()->map()))[17][17]<< endl;
  }
}
void dumpConfigV2(Pds::CsPad::ConfigV2 cfg) {
  printf("in v2 dumper\n");
  cspConfig << "Cspad config V2 dump" << endl; //for detector " << (int) det << endl;
  printf("still in v2 dumper\n");
  cspConfig << "Version: " << CsPad::ConfigV2::Version << endl;
  cspConfig << "Tdi: 0x" << hex << cfg.tdi() << endl;
  cspConfig << "Quad mask: 0x" << hex << cfg.quadMask() << endl;
  cspConfig << "Run delay: 0x" << hex << cfg.runDelay() << endl;
  cspConfig << "Event code: 0x" << hex << cfg.eventCode() << endl;
  cspConfig << "Inactive run mode: 0x" << hex << cfg.inactiveRunMode() << endl;
  cspConfig << "Active run mode: 0x" << hex << cfg.activeRunMode() << endl;
  cspConfig << "Payload size: 0x" << hex << cfg.payloadSize() << endl;
  cspConfig << "Bad asic mask 0: 0x" << hex << cfg.badAsicMask0() << endl;
  cspConfig << "Bad asic mask 1: 0x" << hex << cfg.badAsicMask1() << endl;
  cspConfig << "Asic mask: 0x" << hex << cfg.asicMask() << endl;
  cspConfig << "Number of asics read: " << cfg.numAsicsRead() << endl;
  cspConfig << "Concentrator version: 0x" << cfg.concentratorVersion() << endl;
  for (int j=0; j<4; j++) {
    cspConfig << "Cspad quad " << j << " configuration:" << endl;
    for (int i=0; i<4; i++) {
      cspConfig << "     Shift select " << i << ": 0x" << cfg.quads()[j].shiftSelect()[i]<< endl;
      cspConfig << "     Edge select " << i << ": 0x" << cfg.quads()[j].edgeSelect()[i]<< endl;
    }
    cspConfig << "   Read clock set: 0x" << cfg.quads()[j].readClkSet()<< endl;
    cspConfig << "   Read clock hold: 0x" << cfg.quads()[j].readClkHold()<< endl;
    cspConfig << "   Data mode: 0x" << cfg.quads()[j].dataMode()<< endl;
    cspConfig << "   Prst sel: 0x" << cfg.quads()[j].prstSel()<< endl;
    cspConfig << "   Acq delay: 0x" << cfg.quads()[j].acqDelay()<< endl;
    cspConfig << "   Integration time: 0x" << cfg.quads()[j].intTime()<< endl;
    cspConfig << "   Digital delay: 0x" << cfg.quads()[j].digDelay()<< endl;
    cspConfig << "   Amp idle: 0x" << cfg.quads()[j].ampIdle()<< endl;
    cspConfig << "   Injection total: 0x" << cfg.quads()[j].injTotal()<< endl;
    cspConfig << "   Row col shift per: 0x" << cfg.quads()[j].rowColShiftPer()<< endl;
    cspConfig << "   ReadOnly shift test: 0x" << hex << cfg.quads()[j].ro().shiftTest<< endl;
    cspConfig << "   ReadOnly version: 0x" << hex << cfg.quads()[j].ro().version<< endl;
    for (unsigned i=0; i<CsPad::PotsPerQuad; i++) {
      cspConfig << "   Pot 0x" << i << " value: 0x" << (int)(cfg.quads()[j].dp().value(i))<< endl;
    }
    cspConfig << "Gain map for sample pixel 17/17   : 0x" << (*(cfg.quads()[j].gm()->map()))[17][17]<< endl;
  }
}

void fetchConfig()
{
  if (getCspadConfig(Pds::DetInfo::XppGon, configV1)==0) {
    configVsn= 1;
    lowGain = (*(configV1.quads()[2].gm()->map()))[17][17]==0; // just picks a pixel in quad 2
    dumpConfigV1(configV1);
  }
  else if (getCspadConfig(Pds::DetInfo::XppGon, configV2)==0) {
    configVsn= 2;
    lowGain = (*(configV2.quads()[2].gm()->map()))[17][17]==0;
    printf("call v2 dump\n");
    dumpConfigV2(configV2);
  }
  else {
    configVsn= 0;
    printf("Failed to get CspadConfig\n");
  }
  if(configVsn) printf("*******This is a %s gain run****************\n", lowGain ? "low" : "high");
}

void beginrun() 
{
}

void begincalib()
{
  fetchConfig();
  printf("begincalib\n");
}

void event() 
{
}

void endcalib()
{
}

void endrun() 
{
}

void endjob()
{
}
