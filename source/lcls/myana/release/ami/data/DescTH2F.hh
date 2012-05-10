#ifndef Pds_ENTRYDESCTH2F_HH
#define Pds_ENTRYDESCTH2F_HH

#include "ami/data/DescEntry.hh"

namespace Ami {

  class DescTH2F : public DescEntry {
  public:
    DescTH2F(const char* name, const char* xtitle, const char* ytitle, 
	     unsigned nbinsx, float xlow, float xup,
	     unsigned nbinsy, float ylow, float yup,
	     bool isnormalized=false);

    DescTH2F(const Pds::DetInfo& info,
	     unsigned channel,
	     const char* name, const char* xtitle, const char* ytitle, 
	     unsigned nbinsx, float xlow, float xup,
	     unsigned nbinsy, float ylow, float yup,
	     bool isnormalized=false);

    unsigned nbinsx() const;
    unsigned nbinsy() const;
    float xlow() const;
    float xup() const;
    float ylow() const;
    float yup() const;

    void params(unsigned nbins, float xlow, float xup,
		unsigned nbinsy, float ylow, float yup);

  private:
    uint16_t _nbinsx;
    uint16_t _nbinsy;
    float _xlow;
    float _xup;
    float _ylow;
    float _yup;
  };

  inline unsigned DescTH2F::nbinsx() const {return _nbinsx;}
  inline unsigned DescTH2F::nbinsy() const {return _nbinsy;}
  inline float DescTH2F::xlow() const {return _xlow;}
  inline float DescTH2F::xup() const {return _xup;}
  inline float DescTH2F::ylow() const {return _ylow;}
  inline float DescTH2F::yup() const {return _yup;}
};

#endif
