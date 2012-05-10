#include "LogicAnd.hh"

using namespace Ami;

LogicAnd::LogicAnd(AbsFilter& a,AbsFilter& b) :
  AbsFilter(AbsFilter::LogicAnd),
  _a(a), _b(b)
{
}

LogicAnd::~LogicAnd()
{
  delete &_a;
  delete &_b;
}

bool  LogicAnd::accept() const { return _a.accept() && _b.accept(); }
void* LogicAnd::_serialize(void* p) const
{
  return _b.serialize(_a.serialize(p));
}
AbsFilter* LogicAnd::clone() const 
{
  return new LogicAnd(*_a.clone(),*_b.clone());
}
