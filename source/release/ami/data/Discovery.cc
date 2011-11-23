#include "Discovery.hh"

#include "ami/data/FeatureCache.hh"
#include "ami/data/Cds.hh"
#include "ami/data/DescEntry.hh"

using namespace Ami;

DiscoveryTx::DiscoveryTx(const FeatureCache& f,
			 const Cds&          cds) :
  _features(f),
  _cds     (cds)
{
}

DiscoveryTx::~DiscoveryTx() {}

unsigned DiscoveryTx::niovs() const
{
  return _cds.description()+2;
}

void DiscoveryTx::serialize(iovec* iov) const
{
  _header = _features.entries();
  iov[0].iov_base = &_header;
  iov[0].iov_len  = sizeof(_header);
  iov[1].iov_base = const_cast<char*>(_features.names());
  iov[1].iov_len  = _header*FeatureCache::FEATURE_NAMELEN;
  _cds.description(iov+2);
}


DiscoveryRx::DiscoveryRx(const char* msg, unsigned size) :
  _p(msg), _size(size) 
{
}

DiscoveryRx::~DiscoveryRx() {}

unsigned    DiscoveryRx::features() const
{
  return reinterpret_cast<const uint32_t*>(_p)[0];
}

const char* DiscoveryRx::feature_name(unsigned i) const
{
  const char* name = _p + sizeof(uint32_t);
  return name + i*FeatureCache::FEATURE_NAMELEN;
}
    
const Desc*      DiscoveryRx::title_desc() const
{
  return reinterpret_cast<const Desc*>(feature_name(features()));
}

const DescEntry* DiscoveryRx::entries() const
{
  return reinterpret_cast<const DescEntry*>(title_desc()+1);
}

const DescEntry* DiscoveryRx::end() const
{
  return reinterpret_cast<const DescEntry*>(_p+_size);
}
