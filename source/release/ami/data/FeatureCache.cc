#include "FeatureCache.hh"

#include <string.h>

using namespace Ami;

FeatureCache::FeatureCache() : 
  _entries(0),
  _max_entries(8*sizeof(unsigned)),
  _names  (new char    [_max_entries*FEATURE_NAMELEN]),
  _cache  (new double  [_max_entries]),
  _damaged(new unsigned[_max_entries>>5])
{
}

FeatureCache::~FeatureCache()
{
  delete[] _names;
  delete[] _cache;
  delete[] _damaged;
}

void     FeatureCache::clear()
{
  _entries = 0;
}

unsigned FeatureCache::add(const char* name)
{
  for(unsigned k=0; k<_entries; k++)
    if (strcmp(_names+k*FEATURE_NAMELEN,name)==0)
      return k;

  if (_entries == _max_entries) {
    unsigned max_entries = 2*_max_entries;

    char* names = new char[max_entries*FEATURE_NAMELEN];
    memcpy(names,_names,_max_entries*FEATURE_NAMELEN);
    delete[] _names;
    _names = names;

    double* cache = new double[max_entries];
    memcpy(cache,_cache,_max_entries*sizeof(double));
    delete[] _cache;
    _cache = cache;

    unsigned* damaged = new unsigned[max_entries>>5];
    memcpy(damaged,_damaged,_max_entries>>5);
    delete[] _damaged;
    _damaged = damaged;

    _max_entries = max_entries;
  }
  strncpy(_names + _entries*FEATURE_NAMELEN, name, FEATURE_NAMELEN);
  return _entries++;
}

int         FeatureCache::lookup(const char* name) const
{
  for(int k=0; k<static_cast<int>(_entries); k++)
    if (strcmp(_names+k*FEATURE_NAMELEN,name)==0)
      return k;

  return -1;
}

unsigned    FeatureCache::entries() const { return _entries; }
const char* FeatureCache::names  () const { return _names; }
double      FeatureCache::cache  (int index, bool* damaged) const 
{
  if (damaged) *damaged = (index<0) || ((_damaged[index>>5]>>(index&0x1f)) & 1);
  return index>=0 ? _cache[index] : 0;
}

void        FeatureCache::cache  (int index, double v, bool damaged)
{
  if (index>=0) {
    _cache[index] = v;
    uint32_t mask = 1<<(index&0x1f);
    if (damaged)
      _damaged[index>>5] |= mask;
    else
      _damaged[index>>5] &= ~mask;
  }
}
