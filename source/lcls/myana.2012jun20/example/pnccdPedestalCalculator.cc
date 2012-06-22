/* $Id: pnccdPedestalCalculator.cc,v 1.1 2011/12/05 22:09:20 weaver Exp $ */

#include <TH2.h>
#include <TTree.h>

#include "../myana.hh"
#include "../main.hh"
#include "../release/pdsdata/xtc/DetInfo.hh"

#include <stdio.h>

using namespace std;

static const unsigned PixelVMask = 0x3fff;
static const unsigned Offset=32;
static const unsigned ArraySize=1024*1024/(2*2);
static const unsigned MinCalib=25;

/*
static Pds::DetInfo info[] = { Pds::DetInfo(0,
					    Pds::DetInfo::Camp, 0,
					    Pds::DetInfo::pnCCD, 0),
			       Pds::DetInfo(0,
					    Pds::DetInfo::Camp, 0,
					    Pds::DetInfo::pnCCD, 1),
			       Pds::DetInfo(0,
					    Pds::DetInfo::SxrEndstation, 0,
					    Pds::DetInfo::pnCCD, 0) };
*/

static const char* pfile = 0;

class PixelCalibration {
public:
  enum { NMips=20 };
  PixelCalibration() { reset(); }
public:
  void reset() { _sum=0; for(int i=0; i<NMips; i++) { _low[i]=PixelVMask; _high[i]=0; } }
  void add(unsigned v) { 
    for(unsigned i=0; i<NMips; i++) {
      if (v < _low [i]) { _replace(v, _low , i, PixelVMask); return; }
      if (v > _high[i]) { _replace(v, _high, i,          0); return; }
    }
    _sum += v;
  }
  unsigned avg(unsigned n) { int N = n-2*NMips; return (_sum+(N>>1))/N; }
private:
  void _replace(unsigned v, unsigned* array, unsigned i, unsigned init) 
  {
    if (array[NMips-1]!=init) _sum += array[NMips-1];
    for(unsigned j=NMips-1; j>i; j--)
      array[j] = array[j-1];
    array[i] = v;
  }
private:
  unsigned _sum;
  unsigned _low[NMips], _high[NMips];
};

class MyCCD {
public:
  MyCCD(unsigned info) : _id(info&0xff), _info(info),
                         _pixels(new PixelCalibration[ArraySize]), 
                         _pixelr(new PixelCalibration[ArraySize]), 
                         _offset(new unsigned[ArraySize])
  {}
public:
  void initialize()
  {
    for(unsigned i=0; i<ArraySize; i++) {
      _pixels[i].reset();
      _pixelr[i].reset();
    }
    _n = 0;

    char fname[128];
    if (pfile)
      strcpy(fname,pfile);
    else
      sprintf(fname,"/tmp/ped.%08x.dat",_info);
    FILE* f = fopen(fname,"r");
    if (f) {
      fread(_offset, ArraySize, sizeof(unsigned), f);
      fclose(f);
    }
    else {
      printf("Failed to read %s\n",fname);
      memset(_offset, 0, ArraySize*sizeof(unsigned));
    }

    sprintf(fname,"mean %d",_id);
    _hmean = new TH2F(fname,fname,512,-0.5,511.5,512,-0.5,511.5);
    sprintf(fname,"rms %d",_id);
    _hrms  = new TH2F(fname,fname,512,-0.5,511.5,512,-0.5,511.5);

    sprintf(fname,"nt %d",_id);
    _nt = new TTree(fname,fname);
    _nt->Branch("sector"  , &_sector  , "sector[16]/I", 8000);
  }
  void analyze()
  {
    unsigned char* image;
    int width, height;
    if (getPnCcdValue(_id,image,width,height)==0) {
      const uint16_t* d  = reinterpret_cast<const uint16_t*>(image);
      const uint16_t* d1 = d+width;
      const unsigned*  o = _offset;
      PixelCalibration* c = _pixels;
      PixelCalibration* r = _pixelr;
      for(int iy=0; iy<height; iy+=2,d+=width,d1+=width)
	for(int ix=0; ix<width; ix+=2,c++,r++,o++,d+=2,d1+=2) {
	  unsigned v = 
	    (d [0]&PixelVMask) +
	    (d1[0]&PixelVMask) +
	    (d [1]&PixelVMask) +
	    (d1[1]&PixelVMask);
	  c->add(v);
	  r->add((v-*o)*(v-*o));
	}
      _n++;

      const uint16_t* p = reinterpret_cast<const uint16_t*>(image);
      for(int i=0; i<16; i++) {
	int col =  64 + 128*(i%8);
	int row = 256 + 512*(i/8);
	_sector[i] = p[row*1024+col]&PixelVMask;
      }
      _nt->Fill();
    }
    else
      printf("Failed to retrieve pnccd %d\n",_id);
    
  }
  void finalize  ()
  {
    unsigned* v = new unsigned[ArraySize];
    unsigned* r = new unsigned[ArraySize];
    for(unsigned i=0; i<ArraySize; i++) {
      v[i] = _pixels[i].avg(_n);
      r[i] = _pixelr[i].avg(_n);
    }

    unsigned i=0;
    for(unsigned iy=0; iy<512; iy++)
      for(unsigned ix=0; ix<512; ix++,i++) {
	_hmean->Fill(double(ix),double(iy),double(v[i]));
	_hrms ->Fill(double(ix),double(iy),double(r[i]));
      }

    _hmean->Write();
    _hrms ->Write();
    _nt   ->Write();

    char fname[128];
    if (pfile)
      strcpy(fname,pfile);
    else
      sprintf(fname,"/tmp/ped.%08x.dat",_info);
    FILE* f = fopen(fname,"w");
    if (f) {
      fwrite(v,ArraySize,sizeof(unsigned),f);
      fclose(f);
    }
    else
      printf("Failed to write %s\n",fname);
  }
private:
  int _id;
  unsigned _info;
  unsigned _n;
  PixelCalibration* _pixels;
  PixelCalibration* _pixelr;
  unsigned*         _offset;
  TH2F*             _hmean;
  TH2F*             _hrms ;
  TTree*            _nt;
  int               _sector[16];
};

static MyCCD* ccd;

using namespace Pds;

// This function is called once at the beginning of the analysis job,
// You can ask for detector "configuration" information here.

void beginjob() {
  int fail = 0;

  pfile = getenv("dumpPedestalFile");
  if (pfile) {
    unsigned info = strtoul(strchr(pfile,'.')+1, 0, 16);
    ccd = new MyCCD(info); 
    ccd->initialize();
  }
  else {
    printf("environ var dumpPedestalFile not defined\n");
    exit(1);
  }

  /*
   * Get time information
   */
  int seconds, nanoSeconds;
  getTime( seconds, nanoSeconds );
    
  const char* time;
  fail = getLocalTime( time );

  printf("beginjob\n");
}

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

  ccd->analyze();

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

  ccd->finalize();
}
