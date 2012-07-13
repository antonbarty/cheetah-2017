/* $Id: cspadNoiseCalculator.cc,v 1.3 2012/06/07 16:08:34 weaver Exp $ */
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
#include <cmath>

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

// Below is for frame noise subtraction
static const unsigned fnPixelBins = 16383*2;
static const int fnPixelMin = -16383;
static const int fnPixelMax = 16383;
//static const int fnPeakBins = 3;  // bins on either side of peak used in weighted average
static const int fnPeakBins = 9;  // bins on either side of peak used in weighted average
static const float fnBinScale = float(fnPixelBins)/(fnPixelMax-fnPixelMin);
static const unsigned fnPedestalThreshold = 200; //100; // this is all that matters

class MyNoiseCalculator {
public:
  MyNoiseCalculator(Pds::DetInfo::Detector _detector,
		    Pds::DetInfo::Device _device,
		    int _devId,
		    const char*            _pedPath,
		    const char*            _resPath
		    ) :
    detector   (_detector),
    device   (_device),
    devId (_devId),
    pedFile(new char[strlen(_pedPath)+14]),
    resDumpFile(new char[strlen(_resPath)+14])
  { 
    printf("in noise calc constructor\n");
    memset(pedVal, 0, sizeof(double)*QUADS*TWOXONES*COLS*ROWS);
    memset(resVal, 0, sizeof(double)*QUADS*TWOXONES*COLS*ROWS);
    Pds::DetInfo info(0,_detector,0,_device,devId);
    _quads = QUADS;
    _twoXones = TWOXONES;
    if(device == Pds::DetInfo::Cspad2x2) {
      _quads = 1;
      _twoXones = 2;
    }
    sprintf(resDumpFile,"%s.%08x.dat",_resPath,info.phy());
    printf("calculate noise for %s -> %s\n",
           Pds::DetInfo::name(_detector),
           resDumpFile);

    printf("load peds for %d quads %d 2x1s\n", _quads, _twoXones);
    sprintf(pedFile,"%s.%08x.dat",_pedPath,info.phy());
    try {
      FILE* peds = fopen(pedFile, "r");
      char* linep = NULL;
      size_t sz = 0;
      char* pEnd;
      
      for (unsigned quad=0; quad<_quads; quad++) {
	for (unsigned twoXone=0; twoXone < _twoXones; twoXone++) {
	  for (unsigned col=0; col < COLS; col++) {
	    getline(&linep, &sz, peds);
	    pedVal[quad][twoXone][col][0] = strtod(linep, &pEnd);
	    for (unsigned row=1; row < ROWS; row++) {
	      pedVal[quad][twoXone][col][row] = strtod(pEnd, &pEnd);
	    }
	  }
	  //          if (quad == populatedQuad) {
	  cout << "ped val for 17/17, twoXone " << twoXone << ": " << pedVal[quad][twoXone][17][17] << endl;
	  //          }
	}
      }
      free(linep);
    } catch (...) {
      printf("could not open pedestal file %s, bailing\n", pedFile);
      exit(1);
    }

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
  ~MyNoiseCalculator()
  { 
    if (nEvents==0) {
      printf("found no events to analyze, bailing\n");
      exit(1);
    }
    printf("Attempt to open %s\n", resDumpFile); 
    ofstream dumpedNoise;
    dumpedNoise.open(resDumpFile, ios::out);
    unsigned nQuads = QUADS;
    unsigned n2x1s = TWOXONES;
    if (device == Pds::DetInfo::Cspad2x2) {
      nQuads = 1;
      n2x1s = 2;
    }
    if (dumpedNoise.is_open()) {
      for (unsigned quad=0; quad<nQuads; quad++) {
        for (unsigned twoXone=0; twoXone<n2x1s; twoXone++) {
          for (unsigned col=0; col < COLS; col++) {
            for (unsigned row=0; row < ROWS; row++) {
	      double noise = pow((dataSqrSum[quad][twoXone][col][row]-pow(dataSum[quad][twoXone][col][row],2)/nEvents)/nEvents,0.5);
              dumpedNoise << " " << noise;
            }
            dumpedNoise << endl;
          }
	  //          if (quad==0) printf("Dumping noise val %f for quad 0, 2x1 %d twoXone, col 17, row 17\n", pedVal[0][twoXone][17][17]/nEvents, twoXone);
        }
      }
    }
    else printf("Error opening noise dump file %s\n", resDumpFile);

    delete[] resDumpFile;
  }
public:

  bool getSectionFN(const Pds::CsPad::Section* s, float& fnMean, float& fnRms, unsigned tmpQuad, unsigned tmp2x1) {
    unsigned pixels[fnPixelBins];
    memset(pixels, 0, sizeof(unsigned)*fnPixelBins);
    for(unsigned col=0; col<COLS; col++) {
      for(unsigned row=0; row<ROWS; row++) {
	if (true) {//exceptionalPixels[tmpQuad][tmp2x1][col][row]==0) {
	  float val = s->pixel[col][row] - pedVal[tmpQuad][tmp2x1][col][row];
	  //	  printf("fill or not with %f via %d - %f, should be between %d and %d\n", val, s->pixel[col][row], pedVal[tmpQuad][tmp2x1][col][row], fnPixelMin, fnPixelMax);
	  if (val>fnPixelMin and val<fnPixelMax) {
	    pixels[unsigned((val-fnPixelMin)*fnBinScale)] += 1;
	  }
	}
      }
    }
    //
    unsigned tmpMax = 0;
    unsigned tmpMaxBin = 0;
    for (unsigned i=0; i<fnPixelBins; i++) {
      if (pixels[i]>tmpMax) {
	tmpMax = pixels[i];
	tmpMaxBin = i;
      }
    }
    //  printf("bin %d has %d counts\n", tmpMaxBin, tmpMax);
    
    unsigned thresholdPeakBin = 0;
    unsigned thresholdPeakBinContent = 0;
    double s0 = 0;
    double s1 = 0;
    double s2 = 0;
    for (unsigned i=fnPeakBins; i<fnPixelBins; i++) {
      if ((float)pixels[i]>fnPedestalThreshold) {
	if (pixels[i]<thresholdPeakBinContent) break;
	thresholdPeakBin = i;
	thresholdPeakBinContent = pixels[i];
      }
    }
    
    if (thresholdPeakBinContent == 0) {
      fnMean = 0.;
      fnRms = -1.;
      printf("bailing from fn calculation: max bin %d, content %d, threshold %d\n", tmpMaxBin, tmpMax, fnPedestalThreshold);
      return 1;
    }
    
    for (unsigned j=thresholdPeakBin-fnPeakBins;j<thresholdPeakBin+fnPeakBins+1; j++) {
      s0 += pixels[j];
      s1 += j*pixels[j];
    }
    float binMean = s1/s0;
    fnMean = binMean/fnBinScale + fnPixelMin + 0.5; // 0.5 to make up for bins not being centered
    //  for (unsigned j=thresholdPeakBin-fnPeakBins;j<thresholdPeakBin+fnPeakBins+1; j++) {// obvious one-pass calculation is wrong
    s0 = 0.;
    for (unsigned j=thresholdPeakBin-10;j<thresholdPeakBin+fnPeakBins+1; j++) {// obvious one-pass calculation is wrong
      s0 += pixels[j];
      s2 += pixels[j]*(j-binMean)*(j-binMean);
    }
    fnRms = pow(s2/s0, 0.5)/fnBinScale; // this isn't the standard rms around the mean, but should be similar if rms_real < 3
    // hack for dry run
    //  if (fnMean==0. or fnRms ==0. or not (fnRms < 100 and fnRms> -100)) {
    //    printf("mean %f, rms %f, s0 %f, s1 %f, s2 %f, max bin %d has %d counts, threshold peak bin %d has %d counts\n", fnMean, fnRms, s0, s1, s2, tmpMaxBin, tmpMax, thresholdPeakBin, thresholdPeakBinContent);
    //  }
    return 0;
  }

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
	  float fnMean = 0;
	  float fnRms = 0;
	  //	  if (fnRms>FN_RMS_MAX) fnMean = 0; // bail if too wide

	  getSectionFN(s, fnMean, fnRms, quad, twoXone);
          for(unsigned col=0; col<COLS; col++) {
            for(unsigned row=0; row<ROWS; row++) {
	      double pixelMinusPedMinusFN_peak = s->pixel[col][row] - pedVal[quad][twoXone][col][row] - fnMean;
	      dataSum[quad][twoXone][col][row] += pixelMinusPedMinusFN_peak;
	      dataSqrSum[quad][twoXone][col][row] += pow(pixelMinusPedMinusFN_peak, 2);
            }
          }
        }
      }
      nEvents++;
    } else {
      printf("getCspadData fail %d (%x)\n",fail,fiducials);
    }
  }
private:
  Pds::DetInfo::Detector detector;
  Pds::DetInfo::Device device;
  int devId;

  double                 pedVal[QUADS][TWOXONES][COLS][ROWS];
  double                 resVal[QUADS][TWOXONES][COLS][ROWS];
  double dataSum[QUADS][TWOXONES][COLS][ROWS];
  double dataSqrSum[QUADS][TWOXONES][COLS][ROWS];

  unsigned               nEvents;
  char*                  pedFile;
  char*                  resDumpFile;
  TH1I *pedestalDist[QUADS][TWOXONES];
  unsigned _quads, _twoXones;

};

static list<MyNoiseCalculator*> calculators;


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

  const char* resDumpFile = getenv("dumpNoiseFile");
  if (!resDumpFile) {
    printf("did not find env variable dumpNoiseFile, giving up\n");
    exit(1);
  } else {
    printf("Found env variable dumpNoiseFile with value %s\n", resDumpFile);
  }

  const char* pedFile = getenv("loadPedestalFile");
  if (!pedFile) {
    printf("did not find env variable loadPedestalFile, giving up\n");
    exit(1);
  } else {
    printf("Found env variable loadPedestalFile with value %s\n", pedFile);
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
	      calculators.push_back(new MyNoiseCalculator(detector,device,devId,pedFile,resDumpFile));
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

  for(list<MyNoiseCalculator*>::iterator it = calculators.begin(); 
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
  for(list<MyNoiseCalculator*>::iterator it = calculators.begin(); 
      it != calculators.end(); it++)
    delete (*it);
}
