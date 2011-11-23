#ifndef Pds_DESCPROF_HH
#define Pds_DESCPROF_HH

#include "ami/data/DescEntryW.hh"

namespace Ami {

  class DescProf : public DescEntryW {
  public:
    DescProf(const char* name, const char* xtitle, const char* ytitle, 
	     unsigned nbins, float xlow, float xup, const char* names,
	     const char* weight="");

    DescProf(const Pds::DetInfo& info,
	     unsigned channel,
	     const char* name, const char* xtitle, const char* ytitle, 
	     unsigned nbins, float xlow, float xup, const char* names);

    unsigned nbins() const;
    float xlow() const;
    float xup() const;
    unsigned bin(float x) const;
    const char* names() const;  

    void params(unsigned nbins, float xlow, float xup, const char* names);

  private:
    enum {NamesSize=256};
    char _names[NamesSize];
    uint16_t _nbins;
    uint16_t _unused;
    float _xlow;
    float _xup;
  };

  inline unsigned DescProf::nbins() const {return _nbins;}
  inline float DescProf::xlow() const {return _xlow;}
  inline float DescProf::xup() const {return _xup;}
  inline unsigned DescProf::bin(float x) const {return unsigned((x-_xlow)*_nbins/(_xup-_xlow));}
  inline const char* DescProf::names() const {return _names;}
};

#endif
