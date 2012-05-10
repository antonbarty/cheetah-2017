#include "ami/data/DescCache.hh"

using namespace Ami;

DescCache::DescCache(const char* name, 
                     const char* ytitle,
                     ScalarSet   set ) :
  DescEntry(name, "", ytitle, Cache, sizeof(DescCache)),
  _set     (set)
{}

