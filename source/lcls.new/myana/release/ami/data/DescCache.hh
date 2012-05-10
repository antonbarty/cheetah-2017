#ifndef Pds_DescCache_hh
#define Pds_DescCache_hh

#include "ami/data/DescEntry.hh"
#include "ami/data/FeatureCache.hh"

namespace Ami {
  class DescCache : public DescEntry {
  public:
    DescCache(const char* name, 
              const char* ytitle,
              ScalarSet   set);
  public:
    ScalarSet set() const { return _set; }
  private:
    ScalarSet _set;
  };
};

#endif
