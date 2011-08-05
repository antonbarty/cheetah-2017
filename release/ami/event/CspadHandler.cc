#include "CspadHandler.hh"
#include "CspadAlignment.hh"
#include "CspadAlignment_Commissioning.hh"

#include "ami/event/CspadTemp.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/ChannelID.hh"
#include "ami/data/FeatureCache.hh"

#include "pdsdata/cspad/ElementIterator.hh"
#include "pdsdata/cspad/ElementHeader.hh"
#include "pdsdata/cspad/ConfigV1.hh"
#include "pdsdata/cspad/ConfigV2.hh"
#include "pds/config/CsPadConfigType.hh"
#include "pdsdata/cspad/ElementV2.hh"
#include "pdsdata/xtc/Xtc.hh"

#include <string.h>
#include <stdio.h>
#include <math.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define UNBINNED
#define DO_PED_CORR

typedef Pds::CsPad::ElementV2 CspadElement;

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

//
//  Much of the "template" code which follows is meant to 
//  factor the 90-degree rotations
//

static inline unsigned sum2(const uint16_t*& data)
{ unsigned v = *data++;
  v += *data++; 
  return v; }

static inline unsigned sum4(const uint16_t*& data)
{ unsigned v = *data++;
  v += *data++;
  v += *data++;
  v += *data++;
  return v; }


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
  data++;
  off++;
  gn++;
  return v;
}

static inline unsigned sum2(const uint16_t*& data,
                            const uint16_t*& off,
                            const uint16_t* const*& psp,
                            const double fn,
                            const float*& gn)
{
  unsigned v;
  if (off==*psp) { 
    if (++off==*++psp) psp++;
    off++;
    data+=2; 
    gn += 2;
    v = 2*Offset;
  }
  else {
    double d; 
    d  = (*gn++)*(double(*data++ + *off++ - Offset)-fn);
    d += (*gn++)*(double(*data++ + *off++ - Offset)-fn);
    d += 2*Offset;
    v = unsigned(d+0.5);
  }
  return v; }

static inline unsigned sum4(const uint16_t*& data,
                            const uint16_t*& off,
                            const uint16_t* const*& psp,
                            const double fn,
                            const float*& gn)
{
  unsigned v;
  if (off==*psp) {
    if (++off==*++psp) {
      if (++off==*++psp) {
        if (++off==*++psp) psp++;
        off++;
      }
      else off+=2;
    }
    else off+=3;
    data += 4;
    gn   += 4;
    v = 4*Offset;
  }
  else {
    double d;
    d  = (*gn++)*(double(*data++ + *off++ - Offset)-fn);
    d += (*gn++)*(double(*data++ + *off++ - Offset)-fn);
    d += (*gn++)*(double(*data++ + *off++ - Offset)-fn);
    d += (*gn++)*(double(*data++ + *off++ - Offset)-fn); 
    d += 4*Offset; 
    v = unsigned(d+0.5); }
  return v; }

#if 0
static double frameNoise(const uint16_t*  data,
                         const uint16_t*  off,
                         const uint16_t* const* sta)
{
  double sum = 0;
  const unsigned ColBins = CsPad::ColumnsPerASIC;
  const unsigned RowBins = CsPad::MaxRowsPerASIC<<1;
  const uint16_t* d(data);
  const uint16_t* o(off );
  for(unsigned i=0; i<ColBins; i++) {
    for(unsigned j=0; j<RowBins; j++, d++, o++) {
      int v = *d + *o - Offset;
      sum += double(v);
    }
  }
  return sum/double(ColBins*RowBins);
}
#else
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
      for(unsigned j=0; j<RowBins; j++, d++, o++) {
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
  { const int fnPeakBins = 3;
    const int fnPixelRange = fnPixelBins-fnPeakBins-1;
    const unsigned fnPedestalThreshold = 100;
    
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
      const double allowedPedestalWidthSquared = 2.5*2.5;
      //      printf("frameNoise finds mean %f, variance %f\n", v, double(s2)/double(s0));
      if (double(s2)/double(s0)>allowedPedestalWidthSquared) v = 0; 
      // this isn't the standard rms around the mean, but should be similar if rms_real < 3
      //      printf("frameNoise finds mean %f, variance %f\n", v, double(s2)/double(s0));

    }
    else
      //      printf("frameNoise : peak not found\n");
      ;
  }

  return v;
}
#endif

namespace CspadGeometry {

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


#define BIN_ITER4 {							\
    const unsigned ColBins = CsPad::ColumnsPerASIC>>2;			\
    const unsigned RowBins = CsPad::MaxRowsPerASIC>>1;			\
    /*  zero the target region  */					\
    for(unsigned i=0; i<=ColBins; i++) {				\
      for(unsigned j=0; j<=RowBins; j++) {				\
	const unsigned x = CALC_X(column,i,j);				\
	const unsigned y = CALC_Y(row   ,i,j);				\
	image.content(0,x,y);						\
      }									\
    }									\
    /*  fill the target region  */					\
    for(unsigned i=0; i<ColBins; i++) {					\
      for(unsigned k=0; k<4; k++) {					\
	for(unsigned j=0; j<RowBins; j++) { /* unroll ppb */		\
	  const unsigned x = CALC_X(column,i,j);			\
	  const unsigned y = CALC_Y(row   ,i,j);			\
	  image.addcontent(F4,x,y);                                     \
	}								\
      }									\
    }									\
    for(unsigned j=0; j<RowBins; j++) { /* unroll ppb(y) */		\
      const unsigned x = CALC_X(column,ColBins,j);			\
      const unsigned y = CALC_Y(row   ,ColBins,j);			\
      image.addcontent(4*F4,x,y);                                       \
    }									\
}

#define BIN_ITER2 {							\
    const unsigned ColBins = CsPad::ColumnsPerASIC>>1;			\
    const unsigned RowBins = CsPad::MaxRowsPerASIC;			\
    /*  zero the target region  */					\
    for(unsigned i=0; i<=ColBins; i++) {				\
      for(unsigned j=0; j<=RowBins; j++) {				\
	const unsigned x = CALC_X(column,i,j);				\
	const unsigned y = CALC_Y(row   ,i,j);				\
	image.content(0,x,y);						\
      }									\
    }									\
    /*  fill the target region  */					\
    for(unsigned i=0; i<ColBins; i++) {					\
      for(unsigned k=0; k<2; k++) {					\
	for(unsigned j=0; j<RowBins; j++) { /* unroll ppb */		\
	  const unsigned x = CALC_X(column,i,j);			\
	  const unsigned y = CALC_Y(row   ,i,j);			\
	  image.addcontent(F2,x,y);                                     \
	}								\
      }									\
    }									\
    for(unsigned j=0; j<RowBins; j++) { /* unroll ppb(y) */		\
      const unsigned x = CALC_X(column,ColBins,j);			\
      const unsigned y = CALC_Y(row   ,ColBins,j);			\
      image.addcontent(2*F2,x,y);                                       \
    }									\
}

#define BIN_ITER1 {							\
    const unsigned ColBins = CsPad::ColumnsPerASIC;			\
    const unsigned RowBins = CsPad::MaxRowsPerASIC<<1;			\
    /*  zero the target region  */					\
    for(unsigned i=0; i<=ColBins; i++) {				\
      for(unsigned j=0; j<=RowBins; j++) {				\
	const unsigned x = CALC_X(column,i,j);				\
	const unsigned y = CALC_Y(row   ,i,j);				\
	image.content(0,x,y);						\
      }									\
    }									\
    /*  fill the target region  */					\
    for(unsigned i=0; i<ColBins; i++) {					\
      for(unsigned j=0; j<RowBins; j++) {				\
	const unsigned x = CALC_X(column,i,j);				\
	const unsigned y = CALC_Y(row   ,i,j);				\
	image.addcontent(F1,x,y);					\
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
      ppb(ppbin) {}
    virtual ~Asic() {}
  public:
    virtual void fill(Ami::DescImage& image) const = 0;
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
    void fill(Ami::EntryImage& image,					\
	      const uint16_t*  data) const { bi }                       \
  }

#define F1 (*data++)
#define F2 sum2(data)
#define F4 sum4(data)

#define CALC_X(a,b,c) (a+b)			    
#define CALC_Y(a,b,c) (a-c)			     
  AsicTemplate(  AsicD0B1, BIN_ITER1, 1);
  AsicTemplate(  AsicD0B2, BIN_ITER2, 2);
  AsicTemplate(  AsicD0B4, BIN_ITER4, 4);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a+c)			    
#define CALC_Y(a,b,c) (a+b)			     
  AsicTemplate( AsicD90B1, BIN_ITER1, 1);
  AsicTemplate( AsicD90B2, BIN_ITER2, 2);
  AsicTemplate( AsicD90B4, BIN_ITER4, 4);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a-b)			    
#define CALC_Y(a,b,c) (a+c)			     
  AsicTemplate(AsicD180B1, BIN_ITER1, 1);
  AsicTemplate(AsicD180B2, BIN_ITER2, 2);
  AsicTemplate(AsicD180B4, BIN_ITER4, 4);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a-c)			    
#define CALC_Y(a,b,c) (a-b)			     
  AsicTemplate(AsicD270B1, BIN_ITER1, 1);
  AsicTemplate(AsicD270B2, BIN_ITER2, 2);
  AsicTemplate(AsicD270B4, BIN_ITER4, 4);
#undef CALC_X
#undef CALC_Y

#undef F1
#undef F2
#undef F4
#undef AsicTemplate

  class AsicP : public Asic {
  public:
    AsicP(double x, double y, unsigned ppbin, FILE* ped, FILE* status, FILE* gain) :
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
      
      if (linep) {
        free(linep);
      }
    }
  protected:
    uint16_t  _off[CsPad::MaxRowsPerASIC*CsPad::ColumnsPerASIC*2];
    uint16_t* _sta[CsPad::MaxRowsPerASIC*CsPad::ColumnsPerASIC*2];
    float     _gn [CsPad::MaxRowsPerASIC*CsPad::ColumnsPerASIC*2];
  };

#define AsicTemplate(classname,bi,PPB)					\
  class classname : public AsicP {					\
  public:								\
    classname(double x, double y,                                       \
              FILE* p, FILE* s, FILE* g)                                \
      : AsicP(x,y,PPB,p,s,g) {}                                         \
    void boundary(unsigned& dx0, unsigned& dx1,				\
		  unsigned& dy0, unsigned& dy1) const {			\
      FRAME_BOUNDS;							\
      dx0=x0; dx1=x1; dy0=y0; dy1=y1; }					\
    void fill(Ami::DescImage& image) const {				\
      FRAME_BOUNDS;							\
      image.add_frame(x0,y0,x1-x0+1,y1-y0+1);				\
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
#define F2 sum2(data,off,sta,fn,gn)
#define F4 sum4(data,off,sta,fn,gn)

#define CALC_X(a,b,c) (a+b)			    
#define CALC_Y(a,b,c) (a-c)			     
  AsicTemplate(  AsicD0B1P, BIN_ITER1, 1);
  AsicTemplate(  AsicD0B2P, BIN_ITER2, 2);
  AsicTemplate(  AsicD0B4P, BIN_ITER4, 4);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a+c)			    
#define CALC_Y(a,b,c) (a+b)			     
  AsicTemplate( AsicD90B1P, BIN_ITER1, 1);
  AsicTemplate( AsicD90B2P, BIN_ITER2, 2);
  AsicTemplate( AsicD90B4P, BIN_ITER4, 4);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a-b)			    
#define CALC_Y(a,b,c) (a+c)			     
  AsicTemplate(AsicD180B1P, BIN_ITER1, 1);
  AsicTemplate(AsicD180B2P, BIN_ITER2, 2);
  AsicTemplate(AsicD180B4P, BIN_ITER4, 4);
#undef CALC_X
#undef CALC_Y
#define CALC_X(a,b,c) (a-c)			    
#define CALC_Y(a,b,c) (a-b)			     
  AsicTemplate(AsicD270B1P, BIN_ITER1, 1);
  AsicTemplate(AsicD270B2P, BIN_ITER2, 2);
  AsicTemplate(AsicD270B4P, BIN_ITER4, 4);
#undef CALC_X
#undef CALC_Y

#undef F1
#undef F2
#undef F4
#undef AsicTemplate

  class TwoByTwo {
  public:
    TwoByTwo(double x, double y, unsigned ppb, Rotation r, 
	     const Ami::Cspad::TwoByTwoAlignment& a,
             FILE* f, FILE* s, FILE* g) 
    {
      for(unsigned i=0; i<2; i++) {
	double tx(x), ty(y);
	_transform(tx,ty,a.xAsicOrigin[i<<1],a.yAsicOrigin[i<<1],r);
        if (f) {
          switch(r) {
          case D0: 
            switch(ppb) {
            case 1:       asic[i] = new  AsicD0B1P(tx,ty,f,s,g); break;
            case 2:       asic[i] = new  AsicD0B2P(tx,ty,f,s,g); break;
            default:      asic[i] = new  AsicD0B4P(tx,ty,f,s,g); break;
            } break;
          case D90:
            switch(ppb) {
            case 1:       asic[i] = new  AsicD90B1P(tx,ty,f,s,g); break;
            case 2:       asic[i] = new  AsicD90B2P(tx,ty,f,s,g); break;
            default:      asic[i] = new  AsicD90B4P(tx,ty,f,s,g); break;
            } break;
          case D180:
            switch(ppb) {
            case 1:       asic[i] = new  AsicD180B1P(tx,ty,f,s,g); break;
            case 2:       asic[i] = new  AsicD180B2P(tx,ty,f,s,g); break;
            default:      asic[i] = new  AsicD180B4P(tx,ty,f,s,g); break;
            } break;
          case D270:
            switch(ppb) {
            case 1:       asic[i] = new  AsicD270B1P(tx,ty,f,s,g); break;
            case 2:       asic[i] = new  AsicD270B2P(tx,ty,f,s,g); break;
            default:      asic[i] = new  AsicD270B4P(tx,ty,f,s,g); break;
            } break;
          default:
            break;
          }
        }
        else {
          switch(r) {
          case D0: 
            switch(ppb) {
            case 1:       asic[i] = new  AsicD0B1(tx,ty); break;
            case 2:       asic[i] = new  AsicD0B2(tx,ty); break;
            default:      asic[i] = new  AsicD0B4(tx,ty); break;
            } break;
          case D90:
            switch(ppb) {
            case 1:       asic[i] = new  AsicD90B1(tx,ty); break;
            case 2:       asic[i] = new  AsicD90B2(tx,ty); break;
            default:      asic[i] = new  AsicD90B4(tx,ty); break;
            } break;
          case D180:
            switch(ppb) {
            case 1:       asic[i] = new  AsicD180B1(tx,ty); break;
            case 2:       asic[i] = new  AsicD180B2(tx,ty); break;
            default:      asic[i] = new  AsicD180B4(tx,ty); break;
            } break;
          case D270:
            switch(ppb) {
            case 1:       asic[i] = new  AsicD270B1(tx,ty); break;
            case 2:       asic[i] = new  AsicD270B2(tx,ty); break;
            default:      asic[i] = new  AsicD270B4(tx,ty); break;
            } break;
          default:
            break;
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
    void fill(Ami::EntryImage&           image,
	      const Pds::CsPad::Section* sector,
	      unsigned                   sector_id) const
    {
      asic[sector_id&1]->fill(image,&(sector->pixel[0][0]));
    }
  public:
    Asic* asic[2];
  };

  class Quad {
  public:
    Quad(double x, double y, unsigned ppb, Rotation r, 
         const Ami::Cspad::QuadAlignment& align,
         FILE* pedFile=0, FILE* staFile=0, FILE* gainFile=0)
    {
      static Rotation _tr[] = {  D0  , D90 , D180, D90 ,
				 D90 , D180, D270, D180,
				 D180, D270, D0  , D270,
				 D270, D0  , D90 , D0 };
      for(unsigned i=0; i<4; i++) {
	Ami::Cspad::TwoByTwoAlignment ta(align.twobytwo(i));
	double tx(x), ty(y);
	_transform(tx,ty,ta.xOrigin,ta.yOrigin,r);
	element[i] = new TwoByTwo( tx, ty, ppb, _tr[r*NPHI+i], ta, 
                                   pedFile, staFile, gainFile );
      }
    }
    ~Quad() { for(unsigned i=0; i<4; i++) delete element[i]; }
  public:
    void fill(Ami::DescImage&    image,
              unsigned           mask) const
    {
      for(unsigned i=0; i<4; i++, mask>>=2)
        if (mask&3)
          element[i]->fill(image, mask&3);
    }
    void fill(Ami::EntryImage&             image,
	      Pds::CsPad::ElementIterator& iter) const
    {
      unsigned id;
      const Pds::CsPad::Section* s;
      while( (s=iter.next(id)) ) {
	element[id>>1]->fill(image,s,id);
      }
    }      
  public:
    TwoByTwo* element[4];
  };

  class ConfigCache {
  public:
    ConfigCache(Pds::TypeId type, const void* payload) : 
      _type(type)
    {
      unsigned size;
      switch(type.version()) {
      case 1:  
        { const Pds::CsPad::ConfigV1& c = 
            *reinterpret_cast<const Pds::CsPad::ConfigV1*>(payload); 
          size = sizeof(c);
          _quadMask   = c.quadMask();
          for(unsigned i=0; i<4; i++)
            _roiMask[i] = (_quadMask&(1<<i)) ? 0xff : 0;
          break; }
      case 2:
        { const Pds::CsPad::ConfigV2& c = 
            *reinterpret_cast<const Pds::CsPad::ConfigV2*>(payload); 
          size = sizeof(c);
          _quadMask   = c.quadMask();
          for(unsigned i=0; i<4; i++)
            _roiMask[i] = c.roiMask(i);
          break; }
      default:
        { const CsPadConfigType& c = 
            *reinterpret_cast<const CsPadConfigType*>(payload); 
          size = sizeof(c);
          _quadMask   = c.quadMask();
          for(unsigned i=0; i<4; i++)
            _roiMask[i] = c.roiMask(i);
          break; }
      }
      _payload = new char[size];
      memcpy(_payload,payload,size);
    }
    ConfigCache(const ConfigCache& c) : _type(c._type)
    { 
      unsigned size;
      switch(_type.version()) {
      case 1:  size = sizeof(Pds::CsPad::ConfigV1); break;
      case 2:  size = sizeof(Pds::CsPad::ConfigV2); break;
      default: size = sizeof(CsPadConfigType); break;
      }
      _payload = new char[size];
      memcpy(_payload,c._payload,size);
      _quadMask = c._quadMask;
      for(unsigned i=0; i<4; i++)
        _roiMask[i] = c._roiMask[i];
    }
    ~ConfigCache() 
    { delete[] _payload; }
  public:
    Pds::CsPad::ElementIterator* iter(const Xtc& xtc) const
    {
      Pds::CsPad::ElementIterator* iter;
      switch(_type.version()) {
      case 1: 
        { const Pds::CsPad::ConfigV1& c = 
            *reinterpret_cast<Pds::CsPad::ConfigV1*>(_payload);
          iter = new Pds::CsPad::ElementIterator(c,xtc);
          break; }
      case 2: 
        { const Pds::CsPad::ConfigV2& c = 
            *reinterpret_cast<Pds::CsPad::ConfigV2*>(_payload);
          iter = new Pds::CsPad::ElementIterator(c,xtc);
          break; }
      default:
        { const CsPadConfigType& c = 
            *reinterpret_cast<CsPadConfigType*>(_payload);
          iter = new Pds::CsPad::ElementIterator(c,xtc);
          break; }
      }
      return iter;
    }
  public:
    unsigned quadMask()           const { return _quadMask; }
    unsigned roiMask (unsigned i) const { return _roiMask[i]; }
  private:
    Pds::TypeId _type;
    char*       _payload;
    unsigned    _quadMask;
    unsigned    _roiMask[4];
  };

  class Detector {
  public:
    Detector(const Src& src,
             const ConfigCache& c,
             FILE* f,    // offsets
             FILE* s,    // status
             FILE* g,    // gain
             FILE* gm,   // geometry
             unsigned max_pixels) :
      _src   (src),
      _config(c)
    {
      unsigned smask = 
	(_config.roiMask(0)<< 0) |
	(_config.roiMask(1)<< 8) |
	(_config.roiMask(2)<<16) |
	(_config.roiMask(3)<<24);

      //  Determine layout : binning, origin
      double x,y;

      const Ami::Cspad::QuadAlignment* qalign = qalign_def;
      if (gm) 
        qalign = Ami::Cspad::QuadAlignment::load(gm);

      //
      //  Create a default layout
      //
      _pixels = 2048-256;
      _ppb = 4;
      { const double frame = double(_pixels)*pixel_size;
	x =  0.5*frame;
	y = -0.5*frame;
      }
      quad[0] = new Quad(x,y,_ppb,D0  ,qalign[0]);
      quad[1] = new Quad(x,y,_ppb,D90 ,qalign[1]);
      quad[2] = new Quad(x,y,_ppb,D180,qalign[2]);
      quad[3] = new Quad(x,y,_ppb,D270,qalign[3]);

      //
      //  Test extremes and narrow the focus
      //
      unsigned xmin(_pixels), xmax(0), ymin(_pixels), ymax(0);
      for(unsigned i=0; i<32; i++) {
	if (smask&(1<<i)) {
	  unsigned x0,x1,y0,y1;
	  quad[i>>3]->element[(i>>1)&3]->asic[i&1]->boundary(x0,x1,y0,y1);
	  if (x0<xmin) xmin=x0;
	  if (x1>xmax) xmax=x1;
	  if (y0<ymin) ymin=y0;
	  if (y1>ymax) ymax=y1;
	}
      }

      for(int i=0; i<4; i++)
	delete quad[i];

      int idx = xmax-xmin+1;
      int idy = ymax-ymin+1;
      int pixels = ((idx>idy) ? idx : idy);
      const int bin0 = 4;
      _ppb = 1;
#ifndef UNBINNED
      while((pixels*4/_ppb+2*bin0) > max_pixels)
	_ppb<<=1;
#endif
      x += pixel_size*double(bin0*int(_ppb) - xmin*4);
      y -= pixel_size*double(bin0*int(_ppb) - ymin*4);

      _pixels = pixels*4 + 2*bin0*_ppb;

      quad[0] = new Quad(x,y,_ppb,D0  ,qalign[0],f,s,g);
      quad[1] = new Quad(x,y,_ppb,D90 ,qalign[1],f,s,g);
      quad[2] = new Quad(x,y,_ppb,D180,qalign[2],f,s,g);
      quad[3] = new Quad(x,y,_ppb,D270,qalign[3],f,s,g);
    }
    ~Detector() { for(unsigned i=0; i<4; i++) delete quad[i]; }

    void fill(Ami::DescImage&    image,
	      Ami::FeatureCache& cache) const
    {
      //
      //  The configuration should tell us how many elements to view
      //
      char buff[64];
      _cache = &cache;
      unsigned qmask = _config.quadMask();
      const char* detname = DetInfo::name(static_cast<const DetInfo&>(_src).detector());
      for(unsigned i=0; i<4; i++)
	if (qmask & (1<<i)) {
	  quad[i]->fill(image, _config.roiMask(i));
	  for(unsigned a=0; a<4; a++) {
	    sprintf(buff,"%s:Cspad:Quad[%d]:Temp[%d]",detname,i,a);
	    _feature[4*i+a] = cache.add(buff);
	  }
	}
    }
    void fill(Ami::EntryImage& image,
	      const Xtc&       xtc) const
    {
#ifdef _OPENMP
      Pds::CsPad::ElementIterator*     iters[5];
      int niters=0;
      {
        do {
          iters[niters++] = _config.iter(xtc);
        } while( iter.next() );
      }
      niters--;

      int i;
      Quad* const* quad = this->quad;
      Ami::FeatureCache* cache = _cache;
#pragma omp parallel shared(iters,quad,cache) private(i) num_threads(4)
      {
#pragma omp for schedule(dynamic,1) nowait
        for(i=0; i<niters; i++) {
          const Pds::CsPad::ElementHeader* hdr = iters[i]->next();
          quad[hdr->quad()]->fill(image,*iters[i]); 
          for(int a=0; a<4; a++)
            cache->cache(_feature[4*hdr->quad()+a],
                         CspadTemp::instance().getTemp(hdr->sb_temp(a)));
          delete iters[i];
        }
      }
#else
      Pds::CsPad::ElementIterator* iter = _config.iter(xtc);
      const Pds::CsPad::ElementHeader* hdr;
      while( (hdr=iter->next()) ) {
	quad[hdr->quad()]->fill(image,*iter); 
	for(int a=0; a<4; a++)
	  _cache->cache(_feature[4*hdr->quad()+a],
			CspadTemp::instance().getTemp(hdr->sb_temp(a)));
      }
      delete iter;
#endif
    }
    unsigned ppb() const { return _ppb; }
    unsigned xpixels() { return _pixels; }
    unsigned ypixels() { return _pixels; }
  private:
    Quad* quad[4];
    const Src&  _src;
    ConfigCache _config;
    mutable Ami::FeatureCache* _cache;
    mutable int _feature[16];
    unsigned _ppb;
    unsigned _pixels;
  };
};

using namespace Ami;

CspadHandler::CspadHandler(const Pds::DetInfo& info, FeatureCache& features, unsigned max_pixels) :
  EventHandler(info, Pds::TypeId::Id_CspadElement, Pds::TypeId::Id_CspadConfig),
  _entry(0),
  _unbinned_entry(0),
  _detector(0),
  _unbinned_detector(0),
  _cache(features),
  _max_pixels(max_pixels),
  _options   (0)
{
}

CspadHandler::~CspadHandler()
{
  if (_detector)
    delete _detector;
  if (_unbinned_detector)
    delete _unbinned_detector;
  if (_entry)
    delete _entry;
  if (_unbinned_entry)
    delete _unbinned_entry;
}

unsigned CspadHandler::nentries() const { return _entry ? 1 : 0; }

const Entry* CspadHandler::entry(unsigned i) const { return i==0 ? _entry : 0; }

const Entry* CspadHandler::hidden_entry(unsigned i) const { return i==0 ? _unbinned_entry : 0; }

void CspadHandler::reset() { _entry = 0; _unbinned_entry = 0; }

void CspadHandler::_configure(Pds::TypeId type,const void* payload, const Pds::ClockTime& t)
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

  CspadGeometry::ConfigCache cfg(type,payload);

  _create_entry( cfg,f,s,g,gm, 
                 _detector, _entry, _max_pixels);
#ifndef UNBINNED
  _create_entry( cfg,f,s,g,gm, 
                 _unbinned_detector, _unbinned_entry, 1<<12);
#endif
  if (f ) fclose(f);
  if (s ) fclose(s);
  if (g ) fclose(g);
  if (gm) fclose(gm);
}

void CspadHandler::_create_entry(const CspadGeometry::ConfigCache& cfg,
                                 FILE* f, FILE* s, FILE* g, FILE* gm,
                                 CspadGeometry::Detector*& detector,
                                 EntryImage*& entry, 
                                 unsigned max_pixels) 
{
  if (f ) rewind(f);
  if (s ) rewind(s);
  if (g ) rewind(g);
  if (gm) rewind(gm);

  if (detector)
    delete detector;

  detector = new CspadGeometry::Detector(info(),cfg,f,s,g,gm,max_pixels);

  if (entry) 
    delete entry;

  const unsigned ppb = detector->ppb();
  const DetInfo& det = static_cast<const DetInfo&>(info());
  DescImage desc(det, 0, ChannelID::name(det,0),
                 detector->xpixels()/ppb, detector->ypixels()/ppb, 
                 ppb, ppb);
  desc.set_scale(pixel_size*1e3,pixel_size*1e3);
    
  detector->fill(desc,_cache);

  entry = new EntryImage(desc);
  memset(entry->contents(),0,desc.nbinsx()*desc.nbinsy()*sizeof(unsigned));

  printf("CspadHandler created entry %p\n", entry);
    
  if (f)
    entry->info(Offset*ppb*ppb,EntryImage::Pedestal);
  else
    entry->info(0,EntryImage::Pedestal);
    
  entry->info(0,EntryImage::Normalization);
  entry->invalid();
}


void CspadHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void CspadHandler::_calibrate(Pds::TypeId::Type, const void* payload, const Pds::ClockTime& t) {}

void CspadHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  const Xtc* xtc = reinterpret_cast<const Xtc*>(payload)-1;
  if (_entry) {
    unsigned o = _entry->desc().options();
    if (_options != o) {
      printf("CspadHandler::event options %x -> %x\n", _options, o);
      _options = o;
    }

    _detector->fill(*_entry,*xtc);
    _entry->info(1,EntryImage::Normalization);
    _entry->valid(t);
  }
  if (_unbinned_entry) {
    _unbinned_detector->fill(*_unbinned_entry,*xtc);
    _unbinned_entry->info(1,EntryImage::Normalization);
    _unbinned_entry->valid(t);
  }
}

void CspadHandler::_damaged() 
{
  if (_entry) 
    _entry->invalid(); 
  if (_unbinned_entry) 
    _unbinned_entry->invalid(); 
}
