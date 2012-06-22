/* $Id: cspadPedestalCalculator.cc,v 1.7 2012/06/07 16:08:34 weaver Exp $ */
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "../myana.hh"
#include "../main.hh"
#include "../release/pdsdata/cspad/ConfigV1.hh"
#include "../release/pdsdata/cspad/ConfigV2.hh"
#include "../release/pdsdata/cspad/ConfigV3.hh"
#include "../release/pdsdata/cspad/ConfigV4.hh"
#include "../release/pdsdata/cspad2x2/ConfigV1.hh"
#include "../release/pdsdata/cspad/ElementHeader.hh"
#include "../release/pdsdata/cspad/ElementIterator.hh"

#include <list>

#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TStyle.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TColor.h>
#include <TApplication.h>
#include <TProfile.h>
#include <TGraph.h>

using namespace std;

static const unsigned  ROWS = 194*2;
static const unsigned  COLS = 185;
static const unsigned TWOXONES = 8;
static const unsigned QUADS = 4;

class MyPedCalculator {
public:
  MyPedCalculator(Pds::DetInfo::Detector _detector,
		  Pds::DetInfo::Device _device,
		  int _devId,
                  const char*            _path) :
    detector   (_detector),
    device   (_device),
    devId (_devId),
    pedDumpFile(new char[strlen(_path)+14])
  { 
    memset(pedVal, 0, sizeof(double)*QUADS*TWOXONES*COLS*ROWS);
    Pds::DetInfo info(0,_detector,0,_device,devId);
    sprintf(pedDumpFile,"%s.%08x.dat",_path,info.phy());
    printf("calculate pedestals for %s -> %s\n",
           Pds::DetInfo::name(_detector),
           pedDumpFile);
    printf("make histos\n");
    char detNumber[8];
    sprintf(detNumber, "%08x",info.phy());
    TDirectory* subDir = gFile->mkdir(detNumber);
    subDir->cd();
    for (int quad=0; quad<4; quad++) {
      for (unsigned twoXone=0; twoXone<TWOXONES; twoXone++) {
	stringstream   title, name;
	name.str("");
	name << "raw_q" << dec << setw(1) << setfill('0') << quad << "s" << dec << setw(2) << setfill('0') << twoXone;
	title.str("");
	title << "Raw quad" << dec << setw(1) << setfill('0') << quad << "TwoXone " << dec << setw(2) << setfill('0') << twoXone << " Pedestal subtracted pixel data";
	pedestalDist[quad][twoXone]   = new TH1I(name.str().c_str(),title.str().c_str(), 200, 0, 4000);
      }
    }
  }
  ~MyPedCalculator()
  { 
    if (nPedestalEventsTaken==0) {
      printf("found no events to analyze, bailing\n");
      exit(1);
    }
    printf("Attempt to open %s\n", pedDumpFile); 
    ofstream dumpedPedestals;
    dumpedPedestals.open(pedDumpFile, ios::out);
    unsigned nQuads = QUADS;
    unsigned n2x1s = TWOXONES;
    if (device == Pds::DetInfo::Cspad2x2) {
      nQuads = 1;
      n2x1s = 2;
    }
    if (dumpedPedestals.is_open()) {
      for (unsigned quad=0; quad<nQuads; quad++) {
        for (unsigned twoXone=0; twoXone<n2x1s; twoXone++) {
          for (unsigned col=0; col < COLS; col++) {
            for (unsigned row=0; row < ROWS; row++) {
              dumpedPedestals << " " << pedVal[quad][twoXone][col][row]/nPedestalEventsTaken;
	      pedestalDist[quad][twoXone]->Fill(pedVal[quad][twoXone][col][row]/nPedestalEventsTaken);
            }
            dumpedPedestals << endl;
          }
          if (quad==0) printf("Dumping ped val %f for quad 0, 2x1 %d twoXone, col 17, row 17\n", pedVal[0][twoXone][17][17]/nPedestalEventsTaken, twoXone);
        }
      }
    }
    else printf("Error opening pedestal dump file %s\n", pedDumpFile);

    delete[] pedDumpFile;
  }
public:
  void event(unsigned fiducials) 
  {
    Pds::CsPad::ElementIterator iter;
    int fail = -1;
    if ((device==Pds::DetInfo::Cspad and (fail=getCspadData(detector, iter))==0) 
	or 
	(device==Pds::DetInfo::Cspad2x2 and (fail=getCspad2x2Data(detector, devId, iter))==0))  {
      const Pds::CsPad::ElementHeader* element;
      while( (element=iter.next()) ) {  // loop over elements (quadrants)
        if (device==Pds::DetInfo::Cspad and fiducials != element->fiducials())
          printf("Fiducials %x/%d:%x\n",fiducials,element->quad(),element->fiducials());
        const Pds::CsPad::Section* s;
        unsigned quad = element->quad();
        unsigned twoXone;
        while((s=iter.next(twoXone))) {  // loop over sections (two by one's)
          for(unsigned col=0; col<COLS; col++) {
            for(unsigned row=0; row<ROWS; row++) {  
              pedVal[quad][twoXone][col][row] += s->pixel[col][row];
	      //	      printf("dump: q %d, t %d, c %d, r %d, v %d\n", quad, twoXone, col, row, s->pixel[col][row]);  
            }
          }
        }
      }
      nPedestalEventsTaken++;
    } else {
      printf("getCspadData fail %d (%x)\n",fail,fiducials);
    }
  }
private:
  Pds::DetInfo::Detector detector;
  Pds::DetInfo::Device device;
  int devId;
  double                 pedVal[QUADS][TWOXONES][COLS][ROWS];
  unsigned               nPedestalEventsTaken;
  char*                  pedDumpFile;
  TH1I *pedestalDist[QUADS][TWOXONES];
};

static list<MyPedCalculator*> calculators;


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

  const char* pedDumpFile = getenv("dumpPedestalFile");
  if (!pedDumpFile) {
    printf("did not find env variable dumpPedestalFile, giving up\n");
    exit(1);
  } else {
    printf("Found env variable dumpPedestalFile with value %s\n", pedDumpFile);
  }

  //
  //  Find all possible Cspad detectors
  //
  { 
    Pds::CsPad::ConfigV1 configV1;
    Pds::CsPad::ConfigV2 configV2;
    Pds::CsPad::ConfigV3 configV3;
    Pds::CsPad::ConfigV4 configV4;
    Pds::CsPad2x2::ConfigV1 configV1_2x2;
    for (int isMini=0; isMini<2; isMini++) {
      for(unsigned i=Pds::DetInfo::NoDetector; i<Pds::DetInfo::NumDetector; i++) {
	for (int devId=0; devId<8; devId++) {
	  if (devId>0 and isMini==0) {
	    continue;
	  }
	  Pds::DetInfo::Detector detector = Pds::DetInfo::Detector(i);
	  Pds::DetInfo::Device device = (isMini == 0) ? Pds::DetInfo::Cspad : Pds::DetInfo::Cspad2x2;
	  if (
	      (device==Pds::DetInfo::Cspad and ((getCspadConfig(detector, configV4)==0) ||
						(getCspadConfig(detector, configV3)==0) ||
						(getCspadConfig(detector, configV2)==0) ||
						(getCspadConfig(detector, configV1)==0)))
	      or (device==Pds::DetInfo::Cspad2x2 and getCspad2x2Config(detector, configV3)==0)
	      or (device==Pds::DetInfo::Cspad2x2 and getCspad2x2Config(detector, devId, configV1_2x2)==0)
	      )
	    {
	      calculators.push_back(new MyPedCalculator(detector,device,devId,pedDumpFile));
	    }
	}
      }
    }
  }

  printf("beginjob\n");
}

// This function is called once for each run.  You should check to see
// if detector configuration information has changed.

void beginrun() 
{
  printf("beginrun\n");
}

void begincalib()
{
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

  for(list<MyPedCalculator*>::iterator it = calculators.begin(); 
      it != calculators.end(); it++)
    (*it)->event(fiducials);

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
  for(list<MyPedCalculator*>::iterator it = calculators.begin(); 
      it != calculators.end(); it++)
    delete (*it);
}
