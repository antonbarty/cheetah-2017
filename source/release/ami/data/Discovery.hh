#ifndef Ami_Discovery_hh
#define Ami_Discovery_hh

#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>

namespace Ami {
  class FeatureCache;
  class Cds;
  class Desc;
  class DescEntry;

  class DiscoveryTx {
  public:
    DiscoveryTx(const FeatureCache&, const Cds&);
    ~DiscoveryTx();
  public:
    unsigned niovs() const;
    void     serialize(iovec*) const;
  private:
    const FeatureCache& _features;
    const Cds&          _cds;
    mutable uint32_t    _header;
  };

  class DiscoveryRx {
  public:
    DiscoveryRx(const char* msg, unsigned size);
    ~DiscoveryRx();
  public:
    unsigned    features    () const;
    const char* feature_name(unsigned i) const;
    
    const Desc*      title_desc() const;
    const DescEntry* entries() const;
    const DescEntry* end    () const;
  public:
    const char* payload() const { return _p; }
    unsigned    payload_size() const { return _size; }
  private:
    const char* _p;
    uint32_t    _size;
  };
};
#endif
