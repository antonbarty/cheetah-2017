#include "AbsFilter.hh"

#include <string.h>

using namespace Ami;

void* AbsFilter::serialize(void* p) const
{
  _insert(p, &_type, sizeof(_type));
  return _serialize(p);
}

void  AbsFilter::_insert (void*& p, const void* b, unsigned size) const
{
  char* u = (char*)p;
  memcpy(u, b, size); 
  p = (void*)(u + size);
}

void  AbsFilter::_extract(const char*& p, void* b, unsigned size)
{
  memcpy(b, p, size); 
  p += size;
}

