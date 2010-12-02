#ifndef CspadCorrector_hh
#define CspadCorrector_hh

#include "pdsdata/cspad/ElementIterator.hh"
#include "pdsdata/xtc/DetInfo.hh"

typedef float CspadSection[Pds::CsPad::ColumnsPerASIC][2*Pds::CsPad::MaxRowsPerASIC];

class CspadPixelStatus {
public:
  CspadPixelStatus();
public:
  bool ok(unsigned col, unsigned row) const;
public:
  uint32_t* _map;
};

class CspadCorrector {
public:
  enum { DarkFrameOffset=1,
	 CommonModeNoise=2,
	 PixelGain      =4 };
  //  Constructor with correction list; e.g. 'darkFrameOffset | pixelGain'
  CspadCorrector(Pds::DetInfo::Detector detId,
		 unsigned               devId,
		 unsigned               corrections);
  ~CspadCorrector();
public:
  //  Returns array of corrected pixel values
  CspadSection& apply(const Pds::CsPad::Section& pixels,
		      unsigned section_id,
		      unsigned quad);
  //  Returns status of pixels for this section
  CspadPixelStatus& status(unsigned section_id,
			   unsigned quad);
public:
  //  Load correction constants
  void   loadConstants(unsigned run);
  bool   event();
private:
  void  _processDarkFrames(unsigned run);
  float _getFrameNoise(const Pds::CsPad::Section& input,
		       const CspadSection&        offsets,
		       const CspadPixelStatus&    status);
private:
  Pds::DetInfo _info;
  unsigned     _corrections;
  CspadSection* _offsets;  // current valid dark frame offsets
  CspadSection* _gains  ;  // current valid pixel gains
  CspadSection* _result ;  // we keep ownership of result array
  CspadPixelStatus* _status;
};

#endif
