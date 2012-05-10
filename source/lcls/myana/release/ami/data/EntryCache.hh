#ifndef Pds_EntryCache_hh
#define Pds_EntryCache_hh

#include "ami/data/Entry.hh"
#include "ami/data/DescCache.hh"

#include <math.h>

namespace Ami {
  class FeatureCache;

  class EntryCache : public Entry {
  public:
    EntryCache(const DescCache& desc, FeatureCache* cache);

    virtual ~EntryCache();

    // Implements Entry
    virtual const DescCache& desc() const;
    virtual DescCache& desc();

    void set(double, bool damaged=false);
  private:
    DescCache _desc;

  private:
    FeatureCache* _cache;
    unsigned      _index;
  };

  inline const DescCache& EntryCache::desc() const {return _desc;}
  inline       DescCache& EntryCache::desc()       {return _desc;}

};


#endif
