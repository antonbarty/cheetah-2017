#ifndef Pds_ENTRYFACTORY_HH
#define Pds_ENTRYFACTORY_HH

namespace Ami {

  class Entry;
  class DescEntry;
  class FeatureCache;

  class EntryFactory {
  public:
    static Entry* entry (const DescEntry& desc);
    static void   source(FeatureCache& cache);
  };
};

#endif
