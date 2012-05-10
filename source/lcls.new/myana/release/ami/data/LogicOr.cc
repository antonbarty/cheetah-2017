#include "LogicOr.hh"

using namespace Ami;

LogicOr::LogicOr(AbsFilter& a,AbsFilter& b) :
  AbsFilter(AbsFilter::LogicOr),
  _a(a), _b(b)
{
}

LogicOr::~LogicOr()
{
  delete &_a;
  delete &_b;
}

bool  LogicOr::accept() const { return _a.accept() || _b.accept(); }
void* LogicOr::_serialize(void* p) const
{
  return _b.serialize(_a.serialize(p));
}
AbsFilter* LogicOr::clone() const
{
  return new LogicOr(*_a.clone(),*_b.clone());
}
