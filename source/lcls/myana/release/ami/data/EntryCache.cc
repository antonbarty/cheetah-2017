#include "ami/data/EntryCache.hh"

#include "ami/data/FeatureCache.hh"

using namespace Ami;

EntryCache::~EntryCache() {}

EntryCache::EntryCache(const DescCache& desc, FeatureCache* cache) :
  _desc(desc),
  _cache(cache)
{
  allocate(0);
  if (_cache)
    _index = cache->add(desc.ytitle());
}

void EntryCache::set(double y, bool damaged)
{
  if (_cache) _cache->cache(_index,y,damaged);
}
