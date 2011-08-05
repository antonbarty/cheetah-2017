#ifndef Ami_DescWaveform_HH
#define Ami_DescWaveform_HH

#include "ami/data/DescEntry.hh"

namespace Ami {

  class DescWaveform : public DescEntry {
  public:
    DescWaveform(const Pds::DetInfo& info,
		 unsigned channel,
		 const char* name, 
		 const char* xtitle, 
		 const char* ytitle, 
		 unsigned nbins, 
		 float xlow, 
		 float xup);

    void params(unsigned nbins, float xlow, float xup);

    unsigned nbins() const;
    float xlow() const;
    float xup() const;

  private:
    uint32_t _nbins;
    float _xlow;
    float _xup;
  };

  inline unsigned DescWaveform::nbins() const {return _nbins;}
  inline float DescWaveform::xlow() const {return _xlow;}
  inline float DescWaveform::xup() const {return _xup;}
};

#endif
