#ifndef Ami_FilterFactory_hh
#define Ami_FilterFactory_hh

namespace Ami {
  class AbsFilter;
  class FeatureCache;
  class FilterFactory {
  public:
    FilterFactory();
    FilterFactory(FeatureCache&);
    ~FilterFactory();
  public:
    AbsFilter* deserialize(const char*&) const;
  private:
    FeatureCache* _f;
  };
};

#endif
