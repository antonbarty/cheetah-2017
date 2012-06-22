#include "CspadMiniHandler.hh"
#include "CspadAlignment.hh"
#include "CspadAlignment_Commissioning.hh"

#include "ami/event/CspadTemp.hh"
#include "ami/data/DescImage.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/ChannelID.hh"
#include "ami/data/FeatureCache.hh"
#include "ami/data/PeakFinder.hh"
#include "ami/data/PeakFinderFn.hh"

#include "pdsdata/cspad/MiniElementV1.hh"
#include "pdsdata/xtc/Xtc.hh"

#include <string.h>
#include <stdio.h>
#include <math.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define UNBINNED
#define DO_PED_CORR

typedef Pds::CsPad::MiniElementV1 CspadElement;

static const unsigned Offset = 0x4000;
static const double pixel_size = 110e-6;

enum Rotation { D0, D90, D180, D270, NPHI=4 };

static void _transform(double& x,double& y,double dx,double dy,Rotation r)
{
  switch(r) {
  case D0  :    x += dx; y += dy; break;
  case D90 :    x += dy; y -= dx; break;
  case D180:    x -= dx; y -= dy; break;
  case D270:    x -= dy; y += dx; break;
  default:                        break;
  }
}

static inline unsigned sum1(const uint16_t*& data,
                            const uint16_t*& off,
                            const uint16_t* const*& psp,
                            const double fn,
                            const float*& gn)
{ 
  unsigned v;
  if (off==*psp) { psp++; v = Offset; }
  else { 
    double d = (*gn)*(double(*data + *off - Offset) - fn);
    d += Offset; 
    v = unsigned(d+0.5);
  }
  off++;
  gn++;
  return v;
}

static double frameNoise(const uint16_t*  data,
                         const uint16_t*  off,
                         const uint16_t* const* sta)
{
  const unsigned ColBins = CsPad::ColumnsPerASIC;
  const unsigned RowBins = CsPad::MaxRowsPerASIC<<1;
  const int fnPixelMin = -100 + Offset;
  const int fnPixelMax =  100 + Offset;
  const int fnPixelBins = fnPixelMax - fnPixelMin;
  const int peakSpace   = 5;
  
  //  histogram the pixel values
  unsigned hist[fnPixelBins];
  { memset(hist, 0, fnPixelBins*sizeof(unsigned));
    const uint16_t* d(data);
    const uint16_t* o(off );
    for(unsigned i=0; i<ColBins; i++) {
      for(unsigned j=0; j<RowBins; j++, d+=2, o++) {
        if (*sta == o)
          sta++;
        else {
          int v = *d + *o - fnPixelMin;
          if (v >= 0 && v < int(fnPixelBins))
            hist[v]++;
        }
      }
    }
  }

  double v = 0;
  // the first peak from the left above this is the pedestal
  { const int fnPeakBins = 5;
    const int fnPixelRange = fnPixelBins-fnPeakBins-1;
    const unsigned fnPedestalThreshold = 1000;
    
    unsigned i=fnPeakBins;
    while( int(i)<fnPixelRange ) {
      if (hist[i]>fnPedestalThreshold) break;
      i++;
    }

    unsigned thresholdPeakBin=i;
    unsigned thresholdPeakBinContent=hist[i];
    while( int(++i)<fnPixelRange ) {
      if (hist[i]<thresholdPeakBinContent) {
        if (i > thresholdPeakBin+peakSpace)
          break;
      }
      else {
        thresholdPeakBin = i;
        thresholdPeakBinContent = hist[i];
      }
    }

    i = thresholdPeakBin;
    if ( int(i)+fnPeakBins<=fnPixelRange ) {
      unsigned s0 = 0;
      unsigned s1 = 0;
      for(unsigned j=i-fnPeakBins-1; j<i+fnPeakBins; j++) {
        s0 += hist[j];
        s1 += hist[j]*j;
      }
      
      double binMean = double(s1)/double(s0);
      v =  binMean + fnPixelMin - Offset;
      
      s0 = 0;
      unsigned s2 = 0;
      for(unsigned j=i-10; j<i+fnPeakBins; j++) {
        s0 += hist[j];
	s2 += hist[j]*(j-int(binMean))*(j-int(binMean));
      }
//      const double allowedPedestalWidthSquared = 2.5*2.5;
      //      printf("frameNoise finds mean %f, variance %f\n", v, double(s2)/double(s0));
//      if (double(s2)/double(s0)>allowedPedestalWidthSquared) v = 0;
      // this isn't the standard rms around the mean, but should be similar if rms_real < 3
      //      printf("frameNoise finds mean %f, variance %f\n", v, double(s2)/double(s0));

    }
    else printf("frameNoise : peak not found\n");
//    printf("CspadMiniHandler::frameNoise v=%lf\n", v);
  }

  return v;
}

namespace CspadMiniGeometry {

  //
  //  When filling the image, compensate data which
  //    only partially fills a pixel (at the edges)
  //
#define FRAME_BOUNDS 							\
  const unsigned ColLen   =   CsPad::ColumnsPerASIC/ppb-1;              \
    const unsigned RowLen = 2*CsPad::MaxRowsPerASIC/ppb-1;		\
    unsigned x0 = CALC_X(column,0,0);					\
    unsigned x1 = CALC_X(column,ColLen,RowLen);                         \
    unsigned y0 = CALC_Y(row,0,0);					\
    unsigned y1 = CALC_Y(row,ColLen,RowLen);				\
    if (x0 > x1) { unsigned t=x0; x0=x1; x1=t; }			\
    if (y0 > y1) { unsigned t=y0; y0=y1; y1=t; }			


#define BIN_ITER1 {							\
    const unsigned ColBins = CsPad::ColumnsPerASIC;			\
    const unsigned RowBins = CsPad::MaxRowsPerASIC<<1;			\
    /*  fill the target region  */					\
    for(unsigned i=0; i<ColBins; i++) {					\
      for(unsigned j=0; j<RowBins; j++, data+=2) {                      \
	const unsigned x = CALC_X(column,i,j);				\
	const unsigned y = CALC_Y(row   ,i,j);				\
	image.content(F1,x,y);                                          \
      }									\
    }									\
  }

  //
  //  This class locates the ASIC data to the binned image grid
  //
  class Asic {
  public:
    Asic(double x, double y, unsigned ppbin) :
      column(unsigned( x/pixel_size)/ppbin),
      row   (unsigned(-y/pixel_size)/ppbin),
      ppb(ppbin) 
    {}
    virtual ~Asic() {}
  public:
    virtual void fill(Ami::DescImage& image) const = 0;
    virtual void fill(Ami::EntryImage& image,
		      double, double) const = 0;
    virtual void fill(Ami::EntryImage& image,
		      const uint16_t*  data) const = 0;
  public:
    virtual void boundary(unsigned& x0, unsigned& x1, 
			  unsigned& y0, unsigned& y1) const = 0;
  protected:
    unsigned column, row;
    unsigned ppb;
    uint16_t*  _sta[CsPad::MaxRowsPerASIC*CsPad::ColumnsPerASIC*2];
  };

#define AsicTemplate(classname,bi,PPB)					\
  class classname : public Asic {					\
  public:								\
    classname(double x, double y) : Asic(x,y,PPB) {}                    \
    void boundary(unsigned& dx0, unsigned& dx1,				\
		  unsigned& dy0, unsigned& dy1) const {			\
      FRAME_BOUNDS;							\
      dx0=x0; dx1=x1; dy0=y0; dy1=y1; }					\
    void fill(Ami::DescImage& image) const {				\
      FRAME_BOUNDS;							\
      image.add_frame(x0,y0,x1-x0+1,y1-y0+1);				\
    }									\
    void fill(Ami::EntryImage& image,                                   \
              double v0, double v1) const {                             \
      const unsigned ColBins = CsPad::ColumnsPerASIC;			\
      const unsigned RowBins = CsPad::MaxRowsPerASIC<<1;                \
                                                                        \
      int k=0;                                                          \
      for(unsigned i=0; i<ColBins; i++) {				\
        for(unsigned j=0; j<RowBins; j++,k++) {                         \
          const unsigned x = CALC_X(column,i,j);                        \
          const unsigned y = CALC_Y(row   ,i,j);                        \
          image.content(unsigned(v0),x,y);                              \
        }                                                               \
      }									\
    }									\
    void fill(Ami::EntryImage& image,					\
	      const uint16_t*  data) const { bi }                       \
  }

#define F1 (*data)

#define CALC_X(a,b,c) (a+b)			    
#define CALC_Y(a,b,c) (a-c)			     
  AsicTemplate(  AsicD0B1, BIN_ITER1, 1);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a+c)			    
#define CALC_Y(a,b,c) (a+b)			     
  AsicTemplate( AsicD90B1, BIN_ITER1, 1);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a-b)			    
#define CALC_Y(a,b,c) (a+c)			     
  AsicTemplate(AsicD180B1, BIN_ITER1, 1);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a-c)			    
#define CALC_Y(a,b,c) (a-b)			     
  AsicTemplate(AsicD270B1, BIN_ITER1, 1);
#undef CALC_X
#undef CALC_Y

#undef F1
#undef AsicTemplate

  class AsicP : public Asic {
  public:
    AsicP(double x, double y, unsigned ppbin, 
          FILE* ped, FILE* status, FILE* gain, FILE* sigma) :
      Asic(x,y,ppbin)
    { // load offset-pedestal 
      size_t sz = 8 * 1024;
      char* linep = (char *)malloc(sz);
      char* pEnd;

      if (ped) {
        uint16_t* off = _off;
        for(unsigned col=0; col<CsPad::ColumnsPerASIC; col++) {
          getline(&linep, &sz, ped);
          *off++ = Offset - uint16_t(strtod(linep,&pEnd));
          for (unsigned row=1; row < 2*Pds::CsPad::MaxRowsPerASIC; row++)
            *off++ = Offset - uint16_t(strtod(pEnd, &pEnd));
        }
      }
      else
        memset(_off,0,sizeof(_off));

      if (status) {
        uint16_t*  off = _off;
        uint16_t** sta = _sta;
        for(unsigned col=0; col<CsPad::ColumnsPerASIC; col++) {
          getline(&linep, &sz, status);
          if (strtoul(linep,&pEnd,0)) *sta++ = off;
          off++;
          for (unsigned row=1; row < 2*Pds::CsPad::MaxRowsPerASIC; row++, off++)
            if (strtoul(pEnd,&pEnd,0)) *sta++ = off;
        }
      }
      else
        _sta[0] = 0;

      if (gain) {
        float* gn = _gn;
        for(unsigned col=0; col<CsPad::ColumnsPerASIC; col++) {
          getline(&linep, &sz, gain);
          *gn++ = strtod(linep,&pEnd);
          for (unsigned row=1; row < 2*Pds::CsPad::MaxRowsPerASIC; row++)
            *gn++ = strtod(pEnd,&pEnd);
        }
      }
      else {
        float* gn = _gn;
        for(unsigned col=0; col<CsPad::ColumnsPerASIC; col++) {
          for (unsigned row=0; row < 2*Pds::CsPad::MaxRowsPerASIC; row++)
            *gn++ = 1.;
        }
      }
      
      if (sigma) {
        float* sg = _sg;
        for(unsigned col=0; col<CsPad::ColumnsPerASIC; col++) {
          getline(&linep, &sz, sigma);
          *sg++ = strtod(linep,&pEnd);
          for (unsigned row=1; row < 2*Pds::CsPad::MaxRowsPerASIC; row++)
            *sg++ = strtod(pEnd,&pEnd);
        }
      }
      else {
        float* sg = _sg;
        for(unsigned col=0; col<CsPad::ColumnsPerASIC; col++) {
          for (unsigned row=0; row < 2*Pds::CsPad::MaxRowsPerASIC; row++)
            *sg++ = 0.;
        }
      }
      
      if (linep) {
        free(linep);
      }
    }
  protected:
    uint16_t  _off[CsPad::MaxRowsPerASIC*CsPad::ColumnsPerASIC*2];
    float     _gn [CsPad::MaxRowsPerASIC*CsPad::ColumnsPerASIC*2];
    float     _sg [CsPad::MaxRowsPerASIC*CsPad::ColumnsPerASIC*2];
  };

#define AsicTemplate(classname,bi,PPB)					\
  class classname : public AsicP {					\
  public:								\
    classname(double x, double y,                                       \
              FILE* p, FILE* s, FILE* g, FILE* r)                       \
      : AsicP(x,y,PPB,p,s,g,r) {}                                       \
    void boundary(unsigned& dx0, unsigned& dx1,				\
		  unsigned& dy0, unsigned& dy1) const {			\
      FRAME_BOUNDS;							\
      dx0=x0; dx1=x1; dy0=y0; dy1=y1; }					\
    void fill(Ami::DescImage& image) const {				\
      FRAME_BOUNDS;							\
      image.add_frame(x0,y0,x1-x0+1,y1-y0+1);				\
    }									\
    void fill(Ami::EntryImage& image,                                   \
              double v0, double v1) const {                             \
      const unsigned ColBins = CsPad::ColumnsPerASIC;			\
      const unsigned RowBins = CsPad::MaxRowsPerASIC<<1;                \
                                                                        \
      uint16_t* const* sta = &_sta[0];                                  \
      int k=0;                                                          \
      for(unsigned i=0; i<ColBins; i++) {				\
        for(unsigned j=0; j<RowBins; j++,k++) {                         \
          const unsigned x = CALC_X(column,i,j);                        \
          const unsigned y = CALC_Y(row   ,i,j);                        \
          if (*sta == _off+k) {                                         \
            image.content(-1UL,x,y);                                    \
            sta++;                                                      \
          }                                                             \
          else {                                                        \
            image.content(unsigned(v0 + v1*_sg[k]*_gn[k]),x,y);         \
          }                                                             \
        }                                                               \
      }									\
    }									\
    void fill(Ami::EntryImage& image,					\
	      const uint16_t*  data) const {                            \
      bool lsuppress  = image.desc().options()&1;                       \
      bool lcorrectfn = image.desc().options()&2;                       \
      uint16_t* zero = 0;                                               \
      const uint16_t*  off = _off;                                      \
      const uint16_t* const * sta = lsuppress ? _sta : &zero;           \
      const float* gn = _gn;                                            \
      double fn = lcorrectfn ? frameNoise(data,off,sta) : 0;            \
      bi;                                                               \
    }                                                                   \
  }

#define F1 sum1(data,off,sta,fn,gn)

#define CALC_X(a,b,c) (a+b)			    
#define CALC_Y(a,b,c) (a-c)			     
  AsicTemplate(  AsicD0B1P, BIN_ITER1, 1);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a+c)			    
#define CALC_Y(a,b,c) (a+b)			     
  AsicTemplate( AsicD90B1P, BIN_ITER1, 1);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a-b)			    
#define CALC_Y(a,b,c) (a+c)			     
  AsicTemplate(AsicD180B1P, BIN_ITER1, 1);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a-c)			    
#define CALC_Y(a,b,c) (a-b)			     
  AsicTemplate(AsicD270B1P, BIN_ITER1, 1);
#undef CALC_X
#undef CALC_Y

#undef F1
#undef AsicTemplate

  class TwoByTwo {
  public:
    TwoByTwo(double x, double y, unsigned ppb, Rotation r, 
	     const Ami::Cspad::TwoByTwoAlignment& a,
             FILE* f=0, FILE* s=0, FILE* g=0, FILE* rms=0) 
    {
      for(unsigned i=0; i<2; i++) {
	double tx(x), ty(y);
	_transform(tx,ty,a.xAsicOrigin[i<<1],a.yAsicOrigin[i<<1],r);
        if (f) {
          switch(r) {
          case D0  : asic[i] = new  AsicD0B1P  (tx,ty,f,s,g,rms); break;
          case D90 : asic[i] = new  AsicD90B1P (tx,ty,f,s,g,rms); break;
          case D180: asic[i] = new  AsicD180B1P(tx,ty,f,s,g,rms); break;
          case D270: asic[i] = new  AsicD270B1P(tx,ty,f,s,g,rms); break;
          default  : break;
          }
        }
        else {
          switch(r) {
          case D0  : asic[i] = new  AsicD0B1  (tx,ty); break;
          case D90 : asic[i] = new  AsicD90B1 (tx,ty); break;
          case D180: asic[i] = new  AsicD180B1(tx,ty); break;
          case D270: asic[i] = new  AsicD270B1(tx,ty); break;
          default  : break;
          }
        }
      }
    }
    ~TwoByTwo() {  for(unsigned i=0; i<2; i++) delete asic[i]; }
    void fill(Ami::DescImage& image,
              unsigned        mask) const
    {
      if (mask&1) asic[0]->fill(image);
      if (mask&2) asic[1]->fill(image);
    }
    void fill(Ami::EntryImage& image,
              unsigned mask,
              double v0, double v1) const
    {
      if (mask&1) asic[0]->fill(image,v0,v1);
      if (mask&2) asic[1]->fill(image,v0,v1);
    }
    void fill(Ami::EntryImage&           image,
	      const CspadElement&        element) const
    {
      asic[0]->fill(image,&element.pair[0][0].s0);
      asic[1]->fill(image,&element.pair[0][0].s1);
    }
  public:
    Asic* asic[2];
  };

  class Detector {
  public:
    Detector(const Src& src,
             FILE* f,    // offsets
             FILE* s,    // status
             FILE* g,    // gain
             FILE* rms,  // noise
             FILE* gm,   // geometry
             unsigned max_pixels) :
      _src   (src)
    {
      //  Determine layout : binning, origin
      double x,y;

      Ami::Cspad::TwoByTwoAlignment qalign = qalign_def[0].twobytwo(0);
      if (gm) 
        qalign = Ami::Cspad::QuadAlignment::load(gm)->twobytwo(0);

      //
      //  Create a default layout
      //
      _pixels = 2048-256;
      _ppb = 1;
      { const double frame = double(_pixels)*pixel_size;
	x =  0.5*frame;
	y = -0.5*frame;
      }
      mini = new TwoByTwo(x,y,_ppb,D0,qalign);

      //
      //  Test extremes and narrow the focus
      //
      unsigned xmin(_pixels), xmax(0), ymin(_pixels), ymax(0);
      for(unsigned i=0; i<2; i++) {
        unsigned x0,x1,y0,y1;
        mini->asic[i&1]->boundary(x0,x1,y0,y1);
        if (x0<xmin) xmin=x0;
        if (x1>xmax) xmax=x1;
        if (y0<ymin) ymin=y0;
        if (y1>ymax) ymax=y1;
      }

      delete mini;

      int idx = xmax-xmin+1;
      int idy = ymax-ymin+1;
      int pixels = ((idx>idy) ? idx : idy);
      const int bin0 = 4;
      _ppb = 1;

      x += pixel_size*double(bin0*int(_ppb) - int(xmin));
      y -= pixel_size*double(bin0*int(_ppb) - int(ymin));

      _pixels = pixels + 2*bin0*_ppb;

      mini = new TwoByTwo(x,y,_ppb,D0,qalign, f,s,g,rms);
    }
    ~Detector() { delete mini; }

    void fill(Ami::DescImage&    image,
	      Ami::FeatureCache& cache) const
    {
      char buff[64];
      _cache = &cache;
      const char* detname = DetInfo::name(static_cast<const DetInfo&>(_src));
      mini->fill(image, (1<<2)-1);
      for(unsigned a=0; a<4; a++) {
        sprintf(buff,"%s:Temp[%d]",detname,a);
        _feature[a] = cache.add(buff);
      }
    }
    void fill(Ami::EntryImage& image,
	      const Xtc&       xtc) const
    {
      const CspadElement* elem = reinterpret_cast<const CspadElement*>(xtc.payload());
      mini->fill(image,*elem);
      for(int a=0; a<4; a++)
        _cache->cache(_feature[a],
                      CspadTemp::instance().getTemp(elem->sb_temp(a)));
    }
    void fill(Ami::EntryImage& image, 
              double v0,double v1) const
    {
      mini->fill(image, (1<<2)-1, v0, v1);
    }
    unsigned ppb() const { return _ppb; }
    unsigned xpixels() { return _pixels; }
    unsigned ypixels() { return _pixels; }
  private:
    TwoByTwo* mini;
    const Src&  _src;
    mutable Ami::FeatureCache* _cache;
    mutable int _feature[4];
    unsigned _ppb;
    unsigned _pixels;
  };

  class CspadMiniPFF : public Ami::PeakFinderFn {
  public:
    CspadMiniPFF(FILE* gain,
                 FILE* rms,
                 const Detector*  detector,
                 Ami::DescImage& image) :
      _detector(*detector),
      _nbinsx  (image.nbinsx()),
      _nbinsy  (image.nbinsy()),
      _values  (new Ami::EntryImage(image))
    {
      image.pedcalib (true);
      image.gaincalib(gain!=0);
      image.rmscalib (rms !=0);
    }
    virtual ~CspadMiniPFF()
    {
      delete _values;
    }
  public:
    void     setup(double v0,
                   double v1)
    {
      for(unsigned k=0; k<_nbinsy; k++)
        for(unsigned j=0; j<_nbinsx; j++)
          _values->content(-1UL,j,k);

      _detector.fill(*_values,v0,v1);
    }
    unsigned value(unsigned j, unsigned k) const
    {
      return _values->content(j,k);
    }
    Ami::PeakFinderFn* clone() const { return new CspadMiniPFF(*this); }
  private:
    CspadMiniPFF(const CspadMiniPFF& o) :
      _detector(o._detector),
      _nbinsx  (o._nbinsx),
      _nbinsy  (o._nbinsy),
      _values  (new Ami::EntryImage(o._values->desc())) {}
  private:
    const Detector&  _detector;
    unsigned         _nbinsx;
    unsigned         _nbinsy;
    Ami::EntryImage* _values;
  };
};

using namespace Ami;

static std::list<Pds::TypeId::Type> config_type_list()
{
  std::list<Pds::TypeId::Type> types;
  types.push_back(Pds::TypeId::Id_CspadConfig);
  types.push_back(Pds::TypeId::Id_Cspad2x2Config);
  return types;
}

CspadMiniHandler::CspadMiniHandler(const Pds::DetInfo& info, FeatureCache& features, unsigned max_pixels) :
  EventHandler(info, Pds::TypeId::Id_Cspad2x2Element, config_type_list()),
  _entry(0),
  _detector(0),
  _cache(features),
  _max_pixels(max_pixels),
  _options   (0)
{
}

CspadMiniHandler::~CspadMiniHandler()
{
  if (_detector)
    delete _detector;
  if (_entry)
    delete _entry;
}

unsigned CspadMiniHandler::nentries() const { return _entry ? 1 : 0; }

const Entry* CspadMiniHandler::entry(unsigned i) const { return i==0 ? _entry : 0; }

const Entry* CspadMiniHandler::hidden_entry(unsigned i) const { return 0; }

void CspadMiniHandler::reset() { _entry = 0; }

void CspadMiniHandler::_configure(Pds::TypeId type,const void* payload, const Pds::ClockTime& t)
{
  //
  //  Load pedestals
  //
  const int NameSize=128;
  char oname[NameSize];
  sprintf(oname,"ped.%08x.dat",info().phy());
  FILE* f = fopen(oname,"r");
  if (!f) {
    sprintf(oname,"/reg/g/pcds/pds/cspadcalib/ped.%08x.dat",info().phy());
    f = fopen(oname,"r");
  }
  if (f) 
    printf("Loaded pedestals from %s\n",oname);
  else
    printf("Failed to load pedestals\n");

  sprintf(oname,"sta.%08x.dat",info().phy());
  FILE* s = fopen(oname,"r");
  if (!s) {
    sprintf(oname,"/reg/g/pcds/pds/cspadcalib/sta.%08x.dat",info().phy());
    s = fopen(oname,"r");
  }
  if (s)
    printf("Loaded status map from %s\n",oname);
  else
    printf("Failed to load status map\n");

  sprintf(oname,"gain.%08x.dat",info().phy());
  FILE* g = fopen(oname,"r");
  if (!g) {
    sprintf(oname,"/reg/g/pcds/pds/cspadcalib/gain.%08x.dat",info().phy());
    g = fopen(oname,"r");
  }
  if (g)
    printf("Loaded gain map from %s\n",oname);
  else
    printf("Failed to load gain map\n");

  sprintf(oname,"res.%08x.dat",info().phy());
  FILE* rms = fopen(oname,"r");
  if (!rms) {
    sprintf(oname,"/reg/g/pcds/pds/cspadcalib/res.%08x.dat",info().phy());
    rms = fopen(oname,"r");
  }
  if (rms)
    printf("Loaded noise from %s\n",oname);
  else
    printf("Failed to load noise\n");

  sprintf(oname,"geo.%08x.dat",info().phy());
  FILE* gm = fopen(oname,"r");
  if (!gm) {
    sprintf(oname,"/reg/g/pcds/pds/cspadcalib/geo.%08x.dat",info().phy());
    gm = fopen(oname,"r");
  }
  if (gm)
    printf("Loaded geometry from %s\n",oname);
  else
    printf("Failed to load geometry\n");

  _create_entry( f,s,g,rms,gm, 
                 _detector, _entry, _max_pixels);

  Ami::PeakFinder::register_(info().phy(),   
                             new CspadMiniGeometry::CspadMiniPFF(g,rms,_detector,_entry->desc()));

  if (f ) fclose(f);
  if (s ) fclose(s);
  if (g ) fclose(g);
  if (gm) fclose(gm);
}

void CspadMiniHandler::_create_entry(FILE* f, FILE* s, FILE* g, FILE* rms, FILE* gm,
                                     CspadMiniGeometry::Detector*& detector,
                                     EntryImage*& entry, 
                                     unsigned max_pixels) 
{
  if (f ) rewind(f);
  if (s ) rewind(s);
  if (g ) rewind(g);
  if (gm) rewind(gm);

  if (detector)
    delete detector;

  detector = new CspadMiniGeometry::Detector(info(),f,s,g,rms,gm,max_pixels);

  if (entry) 
    delete entry;

  const unsigned ppb = detector->ppb();
  const DetInfo& det = static_cast<const DetInfo&>(info());
  DescImage desc(det, ChannelID::name(det,0), "photons",
                 detector->xpixels()/ppb, detector->ypixels()/ppb, 
                 ppb, ppb,
                 f!=0, g!=0, false);
  desc.set_scale(pixel_size*1e3,pixel_size*1e3);
    
  detector->fill(desc,_cache);

  entry = new EntryImage(desc);
  memset(entry->contents(),0,desc.nbinsx()*desc.nbinsy()*sizeof(unsigned));

  if (f)
    entry->info(Offset*ppb*ppb,EntryImage::Pedestal);
  else
    entry->info(0,EntryImage::Pedestal);
    
  entry->info(0,EntryImage::Normalization);
  entry->invalid();
}


void CspadMiniHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void CspadMiniHandler::_calibrate(Pds::TypeId::Type, const void* payload, const Pds::ClockTime& t) {}

void CspadMiniHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  const Xtc* xtc = reinterpret_cast<const Xtc*>(payload)-1;
  if (_entry) {
    unsigned o = _entry->desc().options();
    if (_options != o) {
      printf("CspadMiniHandler::event options %x -> %x\n", _options, o);
      _options = o;
    }

    _detector->fill(*_entry,*xtc);
    _entry->info(1,EntryImage::Normalization);
    _entry->valid(t);
  }
}

void CspadMiniHandler::_damaged() 
{
  if (_entry) 
    _entry->invalid(); 
}

