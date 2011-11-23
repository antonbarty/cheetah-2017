#include "FilterFactory.hh"

#include "RawFilter.hh"
#include "SingleShot.hh"
#include "FeatureRange.hh"
#include "LogicAnd.hh"
#include "LogicOr.hh"

#include <stdio.h>

using namespace Ami;

FilterFactory::FilterFactory() : _f(0) {}

FilterFactory::FilterFactory(FeatureCache& f) :
  _f(&f) {}

FilterFactory::~FilterFactory() {}

AbsFilter* FilterFactory::deserialize(const char*& p) const
{
  uint32_t type = (AbsFilter::Type)*reinterpret_cast<const uint32_t*>(p);
  p+=sizeof(uint32_t);

  //  printf("FilterFactory::type %d\n",type);

  AbsFilter* r = 0;
  switch(type) {
  case AbsFilter::Raw          : r = new RawFilter; break;
  case AbsFilter::SingleShot   : r = new SingleShot; break;
  case AbsFilter::FeatureRange : r = new FeatureRange (p,_f); break;
  case AbsFilter::LogicAnd  :  
    { AbsFilter* f0 = deserialize(p);
      AbsFilter* f1 = deserialize(p);
      r = new LogicAnd( *f0, *f1 ); }
    break;
  case AbsFilter::LogicOr  :
    { AbsFilter* f0 = deserialize(p);
      AbsFilter* f1 = deserialize(p);
      r = new LogicOr( *f0, *f1 ); }
    break;
  default:
    printf("FilterFactory: unknown type %d @ %p\n",type,p-sizeof(uint32_t));
    break;
  }
  return r;
}
