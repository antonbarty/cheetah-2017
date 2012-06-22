/* $Id: myana_cspad.cc,v 1.8 2012/02/10 17:13:15 jackp Exp $ */
#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>

#include "../myana.hh"
#include "../main.hh"
#include "../release/pdsdata/cspad/ConfigV1.hh"
#include "../release/pdsdata/cspad/ConfigV2.hh"
#include "../release/pdsdata/cspad/ConfigV3.hh"
#include "../release/pdsdata/cspad/ElementHeader.hh"
#include "../release/pdsdata/cspad/ElementIterator.hh"
#include "CspadTemp.cc"
#include "CspadCorrector.hh"
#include "CspadGeometry.hh"

using namespace std;

static CspadCorrector*      corrector;

static Pds::CsPad::ConfigV1 configV1;
static Pds::CsPad::ConfigV2 configV2;
static Pds::Cspad::ConfigV3 configV3;
static unsigned             configVsn;
static unsigned             quadMask;
static unsigned             asicMask;

static const unsigned  ROWS = 194;
static const unsigned  COLS = 185;

static uint32_t nevents = 0;

//static const Pds::DetInfo::Detector _detector = Pds::DetInfo::XppGon;
static const Pds::DetInfo::Detector _detector = Pds::DetInfo::CxiDs1;

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
  void Fill(unsigned col, unsigned row, int value) { _contents[col*_ny+row] += value; }
  TH2* hist() { 
    unsigned k=0;
    for(unsigned j=0; j<_nx; j++)
      for(unsigned i=0; i<_ny; i++,k++) 
	_h->SetBinContent(j+1,i+1,_contents[k]);
    return _h;
  }
  const int* contents() const { return _contents; }
private:
  TH2F*    _h;
  unsigned _nx;
  unsigned _ny;
  int*     _contents;
};

class NoiseAnalysis {
  enum { HistoryLength=128 };
public:
  NoiseAnalysis(unsigned quad,
                unsigned section,
                unsigned col,
                unsigned row) :
    _quad(quad),
    _sect(section),
    _col (col),
    _row (row),
    _nhist(0),
    _hist (new float [HistoryLength]),
    _acf  (new double[HistoryLength+1]),
    _map  (new double[Pds::CsPad::ColumnsPerASIC*2*Pds::CsPad::MaxRowsPerASIC])
  {
    memset(_acf,0,(HistoryLength+1)*sizeof(double));
    memset(_map,0,Pds::CsPad::ColumnsPerASIC*2*Pds::CsPad::MaxRowsPerASIC*sizeof(double));
  }
  ~NoiseAnalysis()
  {
    delete[] _hist;
    delete[] _acf;
    delete[] _map;
  }
public:
  void event(const CspadSection& v)
  {
    float t = v[_col][_row];

    const unsigned index=_nhist%HistoryLength;

    if (_nhist >= HistoryLength) {
      unsigned k=HistoryLength;
      for(unsigned i=index; i<HistoryLength; i++)
        _acf[k--] += t*_hist[i];
      for(unsigned i=0; i<index; i++)
        _acf[k--] += t*_hist[i];
      _acf[0] += t*t;
    }

    _hist[index] = t;

    const float* p = reinterpret_cast<const float*>(v);
    double* m = _map;
    for(unsigned col=0; col<Pds::CsPad::ColumnsPerASIC; col++)
      for(unsigned row=0; row<Pds::CsPad::MaxRowsPerASIC*2; row++)
        *m++ += t * (*p++);

    _nhist++;
  }
  void write()
  {
    if (_nhist) {

      char buff[64];
      sprintf(buff,"Q%d_S%d_C%d_R%d_hist",
              _quad, _sect, _col, _row);
      TH1F* h = new TH1F(buff,buff,HistoryLength,-0.5,double(HistoryLength)-0.5);
      unsigned k=0;
      for(unsigned i=_nhist%HistoryLength; i<HistoryLength; i++)
        h->SetBinContent(k+1,_hist[k++]);
      for(unsigned i=0; i<_nhist%HistoryLength; i++)
        h->SetBinContent(k+1,_hist[k++]);

      double n = 1./double(_nhist-HistoryLength);
      sprintf(buff,"Q%d_S%d_C%d_R%d_acf",
              _quad, _sect, _col, _row);
      TH1F* a = new TH1F(buff,buff,HistoryLength+1,-0.5,double(HistoryLength)+0.5);
      for(unsigned i=0; i<=HistoryLength; i++)
        a->SetBinContent(i+1,_acf[i]*n);

      n = 1./double(_nhist);
      sprintf(buff,"Q%d_S%d_C%d_R%d_nmap",
              _quad, _sect, _col, _row);
      TH2F* m = new TH2F(buff,buff,
                         Pds::CsPad::ColumnsPerASIC  ,-0.5,double(Pds::CsPad::ColumnsPerASIC  ),
                         Pds::CsPad::MaxRowsPerASIC*2,-0.5,double(Pds::CsPad::MaxRowsPerASIC*2));
      double* p = _map;
      for(unsigned col=0; col<Pds::CsPad::ColumnsPerASIC; col++)
        for(unsigned row=0; row<Pds::CsPad::MaxRowsPerASIC*2; row++)
          m->SetBinContent(col+1,row+1,*p++ * n);
    }
  }
private:
  unsigned _quad,_sect,_col,_row;
  unsigned _nhist;
  float*   _hist;
  double*  _acf;
  double*  _map;
};

class MyQuad {
public:
  MyQuad(unsigned q) : _quad(q), _noise(q,0,5,5) {
    char buff[64];
    for(unsigned i=0; i<16; i++) {
      sprintf(buff,"Q%d_ASIC%d_values",q,i);
      //      _h1[i] = new MyTH1(buff,1<<8,6,0);
      _h1[i] = new MyTH1(buff,1<<9,6,-1000);
      sprintf(buff,"Q%d_ASIC%d_map",q,i);
      _h2[i] = new MyTH2(buff,COLS,ROWS);
    }
  }
public:
  void Fill(Pds::CsPad::ElementIterator& iter) {
    unsigned section_id;
    const Pds::CsPad::Section* s;
    while((s=iter.next(section_id))) {
      CspadSection& f = corrector->apply(*s,section_id,_quad);
      if (section_id==0)
        _noise.event(f);
      unsigned asic1 = section_id<<1;
      unsigned asic2 = asic1 + 1;
      for(unsigned col=0; col<COLS; col++)
	for(unsigned row=0; row<ROWS; row++) {	
	  int v1 = int(f[col][row     ]);
	  int v2 = int(f[col][row+ROWS]);
	  _h1[asic1]->Fill(v1);
	  _h1[asic2]->Fill(v2);
	  _h2[asic1]->Fill(col,row,v1);
	  _h2[asic2]->Fill(col,row,v2);
	}
    }
  }    
  void write() {
    for(unsigned i=0; i<16; i++) {
      _h1[i]->hist();
      _h2[i]->hist();
    }
    _noise.write();
  }
  const CspadSection& section(unsigned section_id)
  {
    const int* asic1 = _h2[(section_id<<1)+0]->contents();
    const int* asic2 = _h2[(section_id<<1)+1]->contents();
    for(unsigned col=0; col<COLS; col++)
      for(unsigned row=0; row<ROWS; row++) {	
	_s[col][row]      = *asic1++;
	_s[col][row+ROWS] = *asic2++;
      }
    return _s;
  }
private:
  unsigned _quad;
  NoiseAnalysis _noise;
  MyTH1* _h1[16];
  MyTH2* _h2[16];
  CspadSection _s;
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

static MyQuad* quads[4];
    
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

  corrector = new CspadCorrector(_detector,0,
				 CspadCorrector::DarkFrameOffset);
  // 0);
				 
  for(unsigned i=0; i<4; i++)
    quads[i] = new MyQuad(i);

  printf("beginjob\n");
}

// This function is called once for each run.  You should check to see
// if detector configuration information has changed.

void fetchConfig()
{
  if (getCspadConfig( _detector, configV1 )==0) {
    configVsn= 1;
    quadMask = configV1.quadMask();
    asicMask = configV1.asicMask();
    printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n",
	   quadMask,asicMask,configV1.runDelay());
    printf("\tintTime %d/%d/%d/%d\n",
	   configV1.quads()[0].intTime(),
	   configV1.quads()[1].intTime(),
	   configV1.quads()[2].intTime(),
	   configV1.quads()[3].intTime());
  }
  else if (getCspadConfig( _detector, configV2 )==0) {
    configVsn= 2;
    quadMask = configV2.quadMask();
    asicMask = configV2.asicMask();
    printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n",
	   quadMask,asicMask,configV2.runDelay());
    printf("\tintTime %d/%d/%d/%d\n",
	   configV2.quads()[0].intTime(),
	   configV2.quads()[1].intTime(),
	   configV2.quads()[2].intTime(),
	   configV2.quads()[3].intTime());
  }
  else if (getCspadConfig( _detector, configV3 )==0) {
    configVsn= 3;
    quadMask = configV3.quadMask();
    asicMask = configV3.asicMask();
    printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n",
       quadMask,asicMask,configV3.runDelay());
    printf("\tintTime %d/%d/%d/%d\n",
       configV3.quads()[0].intTime(),
       configV3.quads()[1].intTime(),
       configV3.quads()[2].intTime(),
       configV3.quads()[3].intTime());
  }
  else {
    configVsn= 0;
    printf("Failed to get CspadConfig\n");
  }
}

void beginrun() 
{
  fetchConfig();
  printf("beginrun\n");

  corrector->loadConstants(getRunNumber());
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
  //
  //  Dump some data from the first events
  //
  if (ievent<10) {
    if (getCspadData(_detector, iter)==0) {
      const Pds::CsPad::ElementHeader* element;
      while( (element=iter.next()) ) {  // loop over elements (quadrants)
	printf("Get Cspad Temp { %3.1f %3.1f %3.1f %3.1f }\n",
	       CspadTemp::instance().getTemp(element->sb_temp(0)),
	       CspadTemp::instance().getTemp(element->sb_temp(1)),
	       CspadTemp::instance().getTemp(element->sb_temp(2)),
	       CspadTemp::instance().getTemp(element->sb_temp(3)));
	
	const Pds::CsPad::Section* s;
	unsigned section_id;
	while( (s=iter.next(section_id)) ) {  // loop over sections (two by one's)
	  printf("           Section %d  { %04x %04x %04x %04x }\n",
		 section_id, s->pixel[0][0], s->pixel[0][1], s->pixel[0][2], s->pixel[0][3]);
	}
      }
    }
  }
  //
  //  Do some real analysis here
  //
  if ((fail=getCspadData(_detector, iter))==0) {
    nevents++;
    const Pds::CsPad::ElementHeader* element;
    while( (element=iter.next()) ) {  // loop over elements (quadrants)
      if (fiducials != element->fiducials())
	printf("Fiducials %x/%d:%x\n",fiducials,element->quad(),element->fiducials());
      quads[element->quad()]->Fill(iter);
    }
  }
  else
    printf("getCspadData fail %d (%x)\n",fail,fiducials);

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

  for(unsigned i=0; i<4; i++)
    quads[i]->write();

  CspadGeometry geom(_detector);

  TTree* nt = new TTree("nt","nt");
  CsVector v;
  unsigned i,s;
  nt->Branch("x", &v[0], "x/D", 8000);
  nt->Branch("y", &v[1], "y/D", 8000);
  nt->Branch("q", &i   , "q/I", 8000);
  nt->Branch("s", &s   , "s/I", 8000);
  for(i=0; i<4; i++) {
    for(s=0; s<8; s++) {
      const SectionGeometry& sg   = geom.section(i,s); 
      for(unsigned col=0; col<COLS; col++)
        for(unsigned row=0; row<2*ROWS; row++) {
          v = sg.pixel_centroid(col,row);
          nt->Fill();
        }
    }
  }

  const unsigned nb = 3400;
  double* array = new double[(nb+2)*(nb+2)];
  //  const double grid_space = 0.5*109.92;
  const double grid_space = 109.92;
  const double grid_size  = 0.5*double(nb)*grid_space;
  CsVector offset; offset[0]=grid_size; offset[1]=grid_size;

  for(unsigned i=0; i<4; i++) {
    if (_detector==Pds::DetInfo::XppGon && i!=2)
      continue;
    for(unsigned s=0; s<8; s++) {
      const CspadSection&    sect = quads[i]->section(s);
      const SectionGeometry& sg   = geom.section(i,s); 
      sg.map(sect, array+nb+3, grid_space, nb+2, offset);
    }
  }

  TH2F* hgeom = new TH2F("geo","geo",nb,-grid_size,grid_size,nb,-grid_size,grid_size);
  hgeom->SetContent(array);

  memset(array,0,(nb+2)*(nb+2)*sizeof(double));
  CspadSection sect;
  { float* p = &sect[0][0];
    float* e = p + sizeof(CspadSection)/sizeof(float);
    while( p < e ) *p++ = 1.; }
  for(unsigned i=0; i<4; i++) {
    if (_detector==Pds::DetInfo::XppGon && i!=2)
      continue;
    for(unsigned s=0; s<8; s++) {
      const SectionGeometry& sg   = geom.section(i,s); 
      sg.map(sect, array+nb+3, grid_space, nb+2, offset);
    }
  }

  TH2F* hnorm = new TH2F("nrm","nrm",nb,-grid_size,grid_size,nb,-grid_size,grid_size);
  hnorm->SetContent(array);

  hgeom->Divide(hgeom,hnorm,1.,double(nevents));
}
