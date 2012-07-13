#include "CspadCorrector.hh"

#include "../main.hh"
#include "pdsdata/ana/XtcRun.hh"

#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/cspad/ElementHeader.hh"
#include "pdsdata/cspad/ElementIterator.hh" // Pds::CsPad::Section

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const unsigned SectionPixels = sizeof(CspadSection)/sizeof(float);
static const unsigned MaxSections=32;

static float _commonModeC, _commonModeP;

CspadPixelStatus::CspadPixelStatus() : 
  _map(new uint32_t[(MaxSections*SectionPixels+0x1f)>>5]) 
{
  memset(_map, 0, ((MaxSections*SectionPixels+0x1f)>>5)*sizeof(unsigned));
}

bool CspadPixelStatus::ok(unsigned col, unsigned row) const
{
  unsigned pixel = col*Pds::CsPad::MaxRowsPerASIC*2 + row;
  return (_map[pixel>>5] & (1<<(pixel&0x1f)))==0;
}


CspadCorrector::CspadCorrector(Pds::DetInfo::Detector detId,
			       unsigned               devId,
			       unsigned               corrections) :
  _info       (0,detId,0,Pds::DetInfo::Cspad,devId),
  _corrections(corrections)
{
  _offsets = new CspadSection[MaxSections];
  _gains   = new CspadSection[MaxSections];
  _result  = new CspadSection[MaxSections];
  _status  = new CspadPixelStatus[MaxSections];

  if (!(_corrections & DarkFrameOffset) &&
      (_corrections & (CommonModeNoise|PixelGain))) {
    printf("CspadCorrector: Adding DarkFrameOffset correction\n");
    _corrections |= DarkFrameOffset;
  }
}

CspadCorrector::~CspadCorrector()
{
  delete[] _offsets;
  delete[] _gains  ;
  delete[] _result ;
}

//  Returns array of corrected pixel values
CspadSection& CspadCorrector::apply(const Pds::CsPad::Section& pixels,
				    unsigned section,
				    unsigned quad)
{
  //
  //  Try to minimize the number of iterations over the data
  //  (since the size of a section is bigger than a typical CPU L1 cache)
  //
  unsigned section_id = quad*8+section;
  float*          d = reinterpret_cast<float*>(_result[section_id]);
  const uint16_t* s = &pixels.pixel[0][0];
  const uint16_t* e = s + SectionPixels;

  if (_corrections & DarkFrameOffset) {
    const float* o = reinterpret_cast<const float*>(_offsets[section_id]);
    if ((_corrections & CommonModeNoise) &&
	(_corrections & PixelGain)) {
      float fn_off = _getFrameNoise(pixels,
				    _offsets[section_id],
				    _status[section_id]);
      const float* g = reinterpret_cast<const float*>(_gains[section_id]);
      while( s < e )
	*d++ = *g++ * (float(*s++)-*o++-fn_off);
    }
    else if (_corrections & CommonModeNoise) {
      float fn_off = _getFrameNoise(pixels,
				    _offsets[section_id],
				    _status [section_id]);
      while( s < e )
	*d++ = float(*s++)-*o++-fn_off;
    }
    else if (_corrections & PixelGain) {
      const float* g = reinterpret_cast<const float*>(_gains[section_id]);
      while( s < e )
	*d++ = *g++ * (float(*s++)-*o++);
    }
    else {
      //
      //  Subtract the offset
      //
      while( s < e )
	*d++ = float(*s++)-*o++;
    }
  }
  else {
    //
    //  Copy the integer input into float output
    //
    while( s < e )
      *d++ = float(*s++);
  }

  return _result[section_id];
}

CspadPixelStatus& CspadCorrector::status(unsigned section_id,
					 unsigned quad)
{
  return _status[quad*8+section_id];
}

//  Load correction constants
void   CspadCorrector::loadConstants(unsigned run)
{
  //
  //  Here we will call methods of myana to retrieve the appropriate
  //  dark frame run and iterate over that run to extract the dark frame
  //  offsets (if selected) and store in _offsets[].
  //
  //  _processDarkFrames(run);

  //
  //  For now, load constants from a file.
  //
  //  Load the constants as instructed from environment variables
  //
  char* statusMap = getenv("loadStatusMap");
  if (statusMap) {
  }

  char* pedFile = getenv("loadPedestalFile");
  if (pedFile) {
    FILE* peds = fopen(pedFile, "r");
    char* linep = NULL;
    size_t sz = 0;
    char* pEnd;
    for (unsigned section=0; section < MaxSections; section++) {
      CspadSection& p = _offsets[section];
      for (unsigned col=0; col < Pds::CsPad::ColumnsPerASIC; col++) {
	getline(&linep, &sz, peds);
	p[col][0] = strtod(linep, &pEnd);
	for (unsigned row=1; row < 2*Pds::CsPad::MaxRowsPerASIC; row++) {
	  p[col][row] = strtod(pEnd, &pEnd);
	}
      }
    }
  }
  
  char* gainFile = getenv("loadGainFile");
  if (gainFile) {
    /*
    FILE* gains = fopen(gainFile, "r");
    char* linep = NULL;
    size_t sz = 0;
    char* pEnd;
    for (unsigned section=0; section < MaxSections; section++) {
      CspadSection& p = _offsets[section];
      for (unsigned col=0; col < Pds::CsPad::ColumnsPerASIC; col++) {
	getline(&linep, &sz, gains);
	p[col][0] = strtod(linep, &pEnd);
	for (unsigned row=1; row < 2*Pds::CsPad::MaxRowsPerASIC; row++) {
	  p[col][row] = strtod(pEnd, &pEnd);
	}
      }
    }
    */
  }
}

#if 0
float CspadCorrector::_getFrameNoise(const uint16_t* pixels,
				     const float*    offsets)
{
  //
  //  Calculate the mean and rms of a distribution that is truncated
  //  at both ends
  //
  
  const int lpSide = 6;
  const int lpPixels = lpSide*lpSide;
  const int lpnTrunc = 5;
  const unsigned int lpStartBase = 17;

  int lpStartRow = lpStartBase;
  int lpStartCol = lpStartBase;

  //
  //  .. incomplete
  //
}
#else
float CspadCorrector::_getFrameNoise(const Pds::CsPad::Section& s,
				     const CspadSection&        o,
				     const CspadPixelStatus&    m)
{
  const unsigned fnPixelBins = 2000;
  const int fnPixelMin = -1000;
  const int fnPixelMax = 1000;
  const int fnPeakBins = 3;  // bins on either side of peak used in weighted average
  const int fnPeakSpace = _fnPeakSpace;
  const float fnBinScale = float(fnPixelBins)/(fnPixelMax-fnPixelMin);
  const unsigned fnPedestalThreshold = 200; //100; // this is all that matters
  // the first peak from the left above this is the pedestal
  // unless one does something fancier

  unsigned pixels[fnPixelBins];
  memset(pixels, 0, sizeof(unsigned)*fnPixelBins);
  for(unsigned col=0; col<Pds::CsPad::ColumnsPerASIC; col++) {
    for(unsigned row=0; row<2*Pds::CsPad::MaxRowsPerASIC; row++) {
      float val = s.pixel[col][row] - o[col][row]; // int here drops information
      if (val>fnPixelMin and val<fnPixelMax) {
	pixels[unsigned((val-fnPixelMin)*fnBinScale)] += 1;
      }
    }
  }

  //  go back and remove bad pixels
  const uint32_t* status_map = m._map;
  const uint32_t* status_end = status_map + (SectionPixels>>5);
  do {
    if (*status_map) {
      uint32_t t(*status_map);
      unsigned pixel = (status_map - m._map)<<5;
      do {
	if (t&1) {
	  unsigned col = pixel/(2*Pds::CsPad::MaxRowsPerASIC);
	  unsigned row = pixel - col*2*Pds::CsPad::MaxRowsPerASIC;
	  int val = s.pixel[col][row] - int(o[col][row]);
	  if (val>fnPixelMin and val<fnPixelMax)
	    pixels[unsigned((val-fnPixelMin)*fnBinScale)]--;
	}
	t>>=1; pixel++;
      } while(t);
    }
  } while( ++status_map < status_end );

  unsigned thresholdPeakBin = 0;
  unsigned thresholdPeakBinContent = 0;
  unsigned s0 = 0;
  unsigned s1 = 0;
  double s2 = 0;
  for (unsigned i=fnPeakBins; i<fnPixelBins; i++) {
    if (pixels[i]>fnPedestalThreshold) {
      if (pixels[i]<thresholdPeakBinContent) {
        if (i > thresholdPeakBin+fnPeakSpace)
          break;
      }
      else {
        thresholdPeakBin = i;
        thresholdPeakBinContent = pixels[i];
      }
    }
  }

  if (thresholdPeakBinContent == 0) {
    // could throw an exception here instead
    _commonModeC = 0;
    return 0;
  }

  for (unsigned j=thresholdPeakBin-fnPeakBins;j<thresholdPeakBin+fnPeakBins+1; j++) {
    s0 += pixels[j];
    s1 += j*pixels[j];
  }
  float binMean = s1/(double)s0;
  float fnMean = binMean/fnBinScale + fnPixelMin + 0.5; // 0.5 to make up for bins not being centered
  for (unsigned j=thresholdPeakBin-fnPeakBins;j<thresholdPeakBin+fnPeakBins+1; j++) {// obvious one-pass calculation is wrong
    s2 += pixels[j]*(j-binMean)*(j-binMean);
  }
  float fnRms = pow(s2/(double)s0, 0.5)/fnBinScale;
  if (fnMean==0. or fnRms ==0. or not (fnRms < 100 and fnRms> -100)) {
    printf("mean %f, rms %f, s0 %d, s1 %d, s2 %f, threshold peak bin %d has %d counts\n", fnMean, fnRms, s0, s1, s2, thresholdPeakBin, thresholdPeakBinContent);
  }
  _commonModeC = fnMean;
  _commonModeP = thresholdPeakBin;
  return fnMean;

}
#endif


bool CspadCorrector::event()
{
  Pds::CsPad::ElementIterator iter;
  if (getCspadData(_info.detector(), iter)==0) {
    const Pds::CsPad::ElementHeader* element;
    while( (element=iter.next()) ) {  // loop over elements (quadrants)
      unsigned quad = element->quad();
      const Pds::CsPad::Section* s;
      unsigned section;
      while((s=iter.next(section))) {
	const uint16_t* d = &s->pixel[0][0];
	float* p = reinterpret_cast<float*>(_offsets[quad*8 + section]);
	float* e = p + SectionPixels;
	while( p < e )
	  *p++ += float(*d++);
      }
    }
    return true;
  }
  return false;
}

void CspadCorrector::_processDarkFrames(unsigned run)
{
  memset(_offsets, 0, MaxSections*sizeof(CspadSection));

  XtcRun* xrun = getDarkFrameRun(run);
  if (!xrun) {
    printf("Failed to retrieve dark frames for run %d\n",run);
    return;
  }

  char* buffer = new char[0x2000000];
  Pds::Dgram* dg = (Pds::Dgram*)buffer;

  Result r = OK;
  unsigned nevent = 0;
  do {
    
    r = xrun->next(dg);
    if (r == Error)
      break;

    //    myLevelIter iter(&(dg->xtc), 0, 0);
    
    if (dg->seq.service() == Pds::TransitionId::L1Accept)
    {
      //      iter.iterate();
      if (event())
        nevent++;
    }
  } while(r==OK);
  
  printf("Processed %d events.\n", nevent);

  delete[] buffer;
  delete xrun;


  if (nevent > 20) {
    float* p = reinterpret_cast<float*>(_offsets);
    float* e = p + SectionPixels*MaxSections;
    float  n = nevent;
    while( p < e )
      *p++ /= n;
  }
}

void  CspadCorrector::setPeakSpace(unsigned s) { _fnPeakSpace=s; }
float CspadCorrector::commonMode () const { return _commonModeC; }
float CspadCorrector::commonModeP() const { return _commonModeP; }
