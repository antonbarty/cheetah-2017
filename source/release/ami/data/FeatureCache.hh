#ifndef Ami_FeatureCache_hh
#define Ami_FeatureCache_hh

#include <stdint.h>

namespace Ami {
  class FeatureCache {
  public:
    enum { FEATURE_NAMELEN=64 };
    FeatureCache();
    ~FeatureCache();
  public:
    void     clear();
    unsigned add(const char* name);
    int      lookup(const char* name) const;
  public:
    unsigned    entries() const;
    const char* names  () const;
    double      cache  (int index, bool* damaged=0) const;
    void        cache  (int index, double, bool damaged=false);
  private:
    unsigned  _entries;
    unsigned  _max_entries;
    char*     _names;
    double*   _cache;
    uint32_t* _damaged;
  };
};

#endif
