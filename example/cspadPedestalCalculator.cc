/* $Id: cspadPedestalCalculator.cc,v 1.1 2010/10/26 21:55:30 philiph Exp $ */
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

static bool lowGain = false;
static const unsigned  ROWS = 194*2;
static const unsigned  COLS = 185;
static const unsigned TWOXONES = 8;
static const unsigned QUADS = 4;
static double pedVal[QUADS][TWOXONES][COLS][ROWS];
static unsigned nPedestalEventsTaken = 0;
static char* pedDumpFile = NULL;

static uint32_t sum = 0;

class MyTH1 {
public:
  MyTH1(const char* name, unsigned int nb, unsigned shift, int off) :
    _h (new TH1F(name,name,nb,
		 -0.5+double(off),
		 double(nb<<shift)-0.5+double(off))),
    _nb (nb),
    _sh (shift),
    _off(off),
    _contents(new unsigned[nb]) {}
public:
  void Fill(int value) { _contents[(value-_off)>>_sh]++; }
  TH1* hist() { 
    for(unsigned i=0; i<_nb; i++) 
      _h->SetBinContent(i+1,_contents[i]);
    return _h;
  }
private:
  TH1F* _h;
  unsigned _nb;
  unsigned _sh;
  int      _off;
  unsigned* _contents;
};

class MyTH2 {
public:
  MyTH2(const char* name, unsigned nx, unsigned ny) :
    _h (new TH2F(name,name,nx,-0.5,double(nx)-0.5,ny,-0.5,double(ny)-0.5)),
    _nx(nx),
    _ny(ny),
    _contents(new int[nx*ny]) {}
public:
  void Fill(unsigned col, unsigned row, int value) { _contents[row*_nx+col] += value; }
  TH2* hist() { 
    unsigned k=0;
    for(unsigned i=0; i<_ny; i++) 
      for(unsigned j=0; j<_nx; j++,k++)
	_h->SetBinContent(j+1,i+1,_contents[k]);
    return _h;
  }
private:
  TH2F*    _h;
  unsigned _nx;
  unsigned _ny;
  int*     _contents;
};

static bool beamOn()
{
  int nfifo = getEvrDataNumber();
  for(int i=0; i<nfifo; i++) {
    unsigned eventCode, fiducial, timestamp;
    if (getEvrData(i,eventCode,fiducial,timestamp)) 
      printf("Failed to fetch evr fifo data\n");
    else if (eventCode==140)
      return true;
  }
  return false;;
}

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

  memset(pedVal, 0, sizeof(double)*QUADS*TWOXONES*COLS*ROWS);
  pedDumpFile = getenv("dumpPedestalFile");
  if (!pedDumpFile) {
    printf("did not find env variable dumpPedestalFile, giving up\n");
    exit(1);
  } else {
    printf("Found env variable dumpPedestalFile with value %s\n", pedDumpFile);
  }

  printf("beginjob\n");
}

// This function is called once for each run.  You should check to see
// if detector configuration information has changed.

void fetchConfig()
{
  if (getCspadConfig( Pds::DetInfo::XppGon, configV1 )==0) {
    configVsn= 1;
    lowGain = (*(configV1.quads()[2].gm()->map()))[17][17]==0; // just picks a pixel in quad 2
  }
  else if (getCspadConfig( Pds::DetInfo::XppGon, configV2 )==0) {
    configVsn= 2;
    lowGain = (*(configV2.quads()[2].gm()->map()))[17][17]==0;
  }
  else {
    configVsn= 0;
    printf("Failed to get CspadConfig\n");
  }
  if(configVsn) printf("*******This is a %s gain run****************\n", lowGain ? "low" : "high");
}

void beginrun() 
{
  printf("beginrun\n");
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

  bool beam = beamOn();
  if (ievent<10)
    printf("Beam %s : \n", beam ? "On":"Off");
  
  double gasdet[4];
  if (getFeeGasDet( gasdet )==0 && ievent<10)
    printf("gasdet %g/%g/%g/%g\n",
	   gasdet[0], gasdet[1], gasdet[2], gasdet[3]);

  Pds::CsPad::ElementIterator iter;
  if ((fail=getCspadData(DetInfo::XppGon, iter))==0) {
    const Pds::CsPad::ElementHeader* element;
    while( (element=iter.next()) ) {  // loop over elements (quadrants)
      if (fiducials != element->fiducials())
	printf("Fiducials %x/%d:%x\n",fiducials,element->quad(),element->fiducials());
      const Pds::CsPad::Section* s;
      unsigned quad = element->quad();
      unsigned twoXone;
      while((s=iter.next(twoXone))) {  // loop over sections (two by one's)
        for(unsigned col=0; col<COLS; col++) {
          for(unsigned row=0; row<ROWS; row++) {  
	    pedVal[quad][twoXone][col][row] += s->pixel[col][row];
	  }
	}
      }
    }
    nPedestalEventsTaken++;
  } else {
    printf("getCspadData fail %d (%x)\n",fail,fiducials);
  }
  ++ievent;
}

void endcalib()
{
}

void endrun() 
{
  printf("User analysis endrun() routine called.\n");
  printf("sum = %x\n",sum);
}

void endjob()
{
  if (nPedestalEventsTaken==0) {
    printf("found no events to analyze, bailing\n");
    exit(1);
  }
  printf("Attempt to open %s\n", pedDumpFile); 
  ofstream dumpedPedestals;
  dumpedPedestals.open(pedDumpFile, ios::out);
  if (dumpedPedestals.is_open()) {
    for (unsigned quad=0; quad < 4; quad++) {
      for (unsigned twoXone=0; twoXone < TWOXONES; twoXone++) {
	for (unsigned col=0; col < COLS; col++) {
	  for (unsigned row=0; row < ROWS; row++) {
	    dumpedPedestals << " " << pedVal[quad][twoXone][col][row]/nPedestalEventsTaken;
	  }
	  dumpedPedestals << endl;
	}
	if (quad==2) printf("Dumping ped val %f for quad 2, 2x1 %d twoXone, col 17, row 17\n", pedVal[2][twoXone][17][17]/nPedestalEventsTaken, twoXone);
      }
    }
  }
  else printf("Error opening pedestal dump file %s\n", pedDumpFile);
}
