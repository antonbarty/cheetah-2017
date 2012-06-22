#include "PnccdHandler.hh"

#include "ami/data/EntryImage.hh"
#include "ami/data/ChannelID.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/FrameV1.hh"

#include <string.h>
#include <stdio.h>
#include <time.h>

static const int PixelsPerBin = 2;

static const unsigned rows = 1024;
static const unsigned cols = 1024;
static const unsigned rows_segment = 512;
static const unsigned cols_segment = 512;

static const unsigned PixelVMask = 0x3fff;
static const unsigned Offset=1024;
static const unsigned ArraySize=rows*cols/(PixelsPerBin*PixelsPerBin);
static const unsigned MinCalib=25;

#define COMMONMODE

namespace Ami {
  class PixelCalibration {
  public:
    enum { NMips=2 };
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
};

using namespace Ami;


PnccdHandler::PnccdHandler(const Pds::DetInfo& info,
			   const FeatureCache& cache) : 
  EventHandler(info, Pds::TypeId::Id_pnCCDframe, Pds::TypeId::Id_pnCCDconfig),
  _cache   (cache),
  _correct (new EntryImage(DescImage(Pds::DetInfo(), (unsigned)0, "PNCCD",
				     cols / PixelsPerBin,
				     rows / PixelsPerBin,
				     PixelsPerBin, PixelsPerBin))),
  _calib   (new PixelCalibration[ArraySize]),
  _collect (false),
  _ncollect(0),
  _entry   (0),
  _tform   (true)
{
}
  
PnccdHandler::~PnccdHandler()
{
  delete[] _calib;
  delete   _correct;
}

unsigned PnccdHandler::nentries() const { return _entry ? 1 : 0; }

const Entry* PnccdHandler::entry(unsigned i) const { return i==0 ? _entry : 0; }

void PnccdHandler::reset() { _entry = 0; }

void PnccdHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  _config = *reinterpret_cast<const Pds::PNCCD::ConfigV1*>(payload);

  //  Load calibration from a file
  memset(_correct->contents(), 0, ArraySize*sizeof(unsigned)); 

  const int NameSize=128;
  char oname[NameSize];
  sprintf(oname,"ped.%08x.dat",info().phy());
  FILE* f = fopen(oname,"r");
  if (!f) {
    printf("Failed to open %s\n",oname);
    sprintf(oname,"/reg/g/pcds/pds/pnccdcalib/ped.%08x.dat",info().phy());
    f = fopen(oname,"r");
  }
  if (f) {
    printf("Loading pedestals from %s\n",oname);
    fread(_correct->contents(), sizeof(unsigned), ArraySize, f);
    fclose(f);
  }
  else {
    printf("Failed to load pedestals %s\n",oname);
  }

  sprintf(oname,"rot.%08x.dat",info().phy());
  f = fopen(oname,"r");
  if (!f) {
    sprintf(oname,"/reg/g/pcds/pds/pnccdcalib/rot.%08x.dat",info().phy());
    f = fopen(oname,"r");
  }
  if (f) {
    _tform = false;
    fclose(f);
  }
  else
    _tform = true;

  const Pds::DetInfo& det = static_cast<const Pds::DetInfo&>(info());
  DescImage desc(det, (unsigned)0, ChannelID::name(det),
		 cols / PixelsPerBin,
		 rows / PixelsPerBin,
		 PixelsPerBin, PixelsPerBin);
  _entry = new EntryImage(desc);
}

void PnccdHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}

void PnccdHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  //
  //  Determine if a dark frame run has begun or ended
  //

  const Pds::PNCCD::FrameV1* f = reinterpret_cast<const Pds::PNCCD::FrameV1*>(payload);
  if (!_entry) return;

  _fillQuadrant (f->data(), 0, 0);                      // upper left
  f = f->next(_config);
  _fillQuadrantR(f->data(), cols_segment-1, rows-1);    // lower left
  f = f->next(_config);
  _fillQuadrantR(f->data(), cols-1, rows-1);            // lower right
  f = f->next(_config);
  _fillQuadrant (f->data(), cols_segment, 0);           // upper right
  f = f->next(_config);

  _entry->info(Offset*PixelsPerBin*PixelsPerBin,EntryImage::Pedestal);
  //      _entry->info(0,EntryImage::Pedestal);
  _entry->info(1,EntryImage::Normalization);
  _entry->valid(t);

  _ncollect++;

}

void PnccdHandler::_damaged() { _entry->invalid(); }

void PnccdHandler::_begin_calib() 
{
  for(unsigned i=0; i<ArraySize; i++)
    _calib[i].reset();
  _collect =true;
  _ncollect=0;
}

void PnccdHandler::_end_calib()
{
  _collect=false;
  if (_ncollect >= MinCalib) {

    // compute, store to a file 
    for(unsigned i=0; i<ArraySize; i++) {
      unsigned iy = i>>9;
      unsigned ix = i&0x1ff;
      _correct->content(_calib[i].avg(_ncollect), ix, iy);
    }

    //  Rename the old calibration file
    const int NameSize=128;
    char oname[NameSize], nname[NameSize];
    sprintf(oname,"/tmp/pnccd.%08x.dat",info().phy());
    sprintf(nname,"/tmp/pnccd.%08x.dat.",info().phy());
    time_t t = time(NULL);
    char* dstr = nname+strlen(nname);
    strftime(dstr, nname+NameSize-1-dstr, "%Y%m%D_%H%M%S", localtime(&t));
    rename(oname,nname);

    //  Store the new calibration file
    FILE* f = fopen(oname,"w");
    if (f) {
      fwrite(_correct->contents(), sizeof(unsigned), ArraySize, f);
      fclose(f);
    }
    else
      printf("Failed to write %s\n",oname);
  }
}

void PnccdHandler::_fillQuadrant(const uint16_t* d, unsigned x, unsigned y)
{
  //  Common mode
  int32_t common[256];
#ifdef COMMONMODE
  for(unsigned ix=0; ix<256; ) {
    for(unsigned i=0; i<8; i++,ix++) {
      const uint16_t* p  = 2*512 + (ix<<1) + d;
      const uint16_t* p1 = 3*512 + (ix<<1) + d;
      int32_t v = 0;
      for(unsigned iy=y; iy<16+y; iy++, p+=2*512, p1+=2*512) {
        v += 
          (p [0]&0x3fff) +
          (p [1]&0x3fff) +
          (p1[0]&0x3fff) +
          (p1[1]&0x3fff);
        v -= _correct->content(ix+(x>>1),iy);
      }
      common[ix] = v/16;
    }
  }
#else
  memset(common, 0, 256*sizeof(int32_t));
#endif

  //  PixelsPerBin = 2
  unsigned iy = y>>1;
  for(unsigned j=0; j<rows_segment; j+=2,iy++,d+=cols_segment) {
    const uint16_t* d1 = d+cols_segment;
    unsigned ix = x>>1;
    for(unsigned k=0; k<cols_segment; k+=2,ix++) {
      unsigned v =
	(d [0]&0x3fff) + 
	(d [1]&0x3fff) + 
	(d1[0]&0x3fff) +
	(d1[1]&0x3fff);

//       if (_collect)
// 	_calib[iy*(cols>>1)+ix].add(v);

      v += Offset<<2;
      v -= _correct->content(ix, iy);
      v -= common[ix-(x>>1)];
      if (_tform)
        _entry->content(v, iy, 511-ix);
      else
        _entry->content(v, ix, iy);
      d  += 2;
      d1 += 2;
    }
  }
}

void PnccdHandler::_fillQuadrantR(const uint16_t* d, unsigned x, unsigned y)
{
  //  Common mode
  int32_t common[256];
#ifdef COMMONMODE
  for(unsigned ix=0; ix<256; ) {
    for(unsigned i=0; i<8; i++,ix++) {
      const uint16_t* p  = 2*512 + (ix<<1) + d;
      const uint16_t* p1 = 3*512 + (ix<<1) + d;
      int32_t v = 0;
      for(unsigned iy=(y>>1)-1; iy>(y>>1)-17; iy--, p+=2*512, p1+=2*512) {
        v += 
          (p [0]&0x3fff) +
          (p [1]&0x3fff) +
          (p1[0]&0x3fff) +
          (p1[1]&0x3fff);
        v -= _correct->content((x>>1)-ix,iy);
      }
      common[ix] = v/16;
    }
  }
#else
  memset(common, 0, 256*sizeof(int32_t));
#endif
  //  PixelsPerBin = 2
  unsigned iy = y>>1;
  for(unsigned j=0; j<rows_segment; j+=2,iy--,d+=cols_segment) {
    const uint16_t* d1 = d+cols_segment;
    unsigned ix = x>>1;
    for(unsigned k=0; k<cols_segment; k+=2,ix--) {
      unsigned v = 
	(d [0]&0x3fff) + 
	(d [1]&0x3fff) + 
	(d1[0]&0x3fff) +
	(d1[1]&0x3fff);

//       if (_collect)
// 	_calib[iy*(cols>>1)+ix].add(v);

      v += Offset<<2;
      v -= _correct->content(ix, iy);
      v -= common[(x>>1)-ix];
      if (_tform)
        _entry->content(v, iy, 511-ix);
      else
        _entry->content(v, ix, iy);
      d  += 2;
      d1 += 2;
    }
  }
}

