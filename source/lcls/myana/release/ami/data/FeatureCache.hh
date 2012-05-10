#ifndef Ami_FeatureCache_hh
#define Ami_FeatureCache_hh

#include <stdint.h>
#include <vector>
#include <string>

namespace Ami {

  enum ScalarSet { PreAnalysis, PostAnalysis, NumberOfSets };

  class FeatureCache {
  public:
    enum { FEATURE_NAMELEN=64 };
    FeatureCache();
    ~FeatureCache();
  public:
    void     clear ();
    unsigned add   (const std::string&);
    unsigned add   (const char* name);
    int      lookup(const char* name) const;
    bool     update();
  public:
    unsigned    entries() const;
    const std::vector<std::string>& names() const;
    double      cache  (int index, bool* damaged=0) const;
    void        cache  (int index, double, bool damaged=false);
  public:
    char*  serialize(int& len) const;
  private:
    std::vector<std::string> _names;
    std::vector<double>      _cache;
    std::vector<uint32_t>    _damaged;
    bool                     _update;
  };
};

#endif
