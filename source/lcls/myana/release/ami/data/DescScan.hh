#ifndef Pds_DescScan_hh
#define Pds_DescScan_hh

//
//  Class DescScan 
//    Descriptor for a scatter plot or non-uniform histogram.
//

#include "ami/data/DescEntryW.hh"

namespace Ami {

  class DescScan : public DescEntryW {
  public:
    DescScan(const char* name, const char* xtitle, const char* ytitle, unsigned nbins,
	     const char* weight="", bool scatter=true);

    DescScan(const Pds::DetInfo& info,
 	     unsigned channel,
 	     const char* name, const char* xtitle, const char* ytitle, unsigned nbins,
             bool scatter=true);

    unsigned nbins() const;

    void params(unsigned nbins);
    bool scatter() const;
  private:
    uint16_t _nbins;
    uint16_t _details;
  };

  inline unsigned    DescScan::nbins   () const { return _nbins; }
  inline bool        DescScan::scatter () const { return (_details&1); }
};

#endif
