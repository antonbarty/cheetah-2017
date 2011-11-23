#ifndef Pds_DescScan_hh
#define Pds_DescScan_hh

#include "ami/data/DescEntryW.hh"

namespace Ami {

  class DescScan : public DescEntryW {
  public:
    DescScan(const char* name, const char* xtitle, const char* ytitle, unsigned nbins,
	     const char* weight="");

    DescScan(const Pds::DetInfo& info,
 	     unsigned channel,
 	     const char* name, const char* xtitle, const char* ytitle, unsigned nbins);

    unsigned nbins() const;

    void params(unsigned nbins);

  private:
    uint16_t _nbins;
    uint16_t _unused;
  };

  inline unsigned    DescScan::nbins   () const { return _nbins; }
};

#endif
