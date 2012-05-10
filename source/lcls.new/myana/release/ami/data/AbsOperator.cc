#include "AbsOperator.hh"

#include <string.h>

using namespace Ami;

AbsOperator::AbsOperator(AbsOperator::Type t) : _type(t), _next(0) {}

AbsOperator::Type AbsOperator::type() const { return (AbsOperator::Type)_type; }

AbsOperator*      AbsOperator::next() const { return _next; }

void                   AbsOperator::next(AbsOperator* o) { _next=o; }

Entry&                 AbsOperator::operator()(const Entry& i) const
{
  Entry& o = _operate(i);
  return _next ? _next->_operate(o) : o;
}

void*                  AbsOperator::serialize(void* p) const 
{ 
  static const uint32_t zero(0), one(1);
  _insert(p, &_type, sizeof(_type));
  _insert(p, _next ? &one : &zero, sizeof(uint32_t));
  p = _serialize(p);
  return _next ? _next->serialize(p) : p;
}

void AbsOperator::_insert(void*& p, const void* b, unsigned size) const 
{
  char* u = (char*)p;
  memcpy(u, b, size); 
  p = (void*)(u + size);
}

void AbsOperator::_extract(const char*& p, void* b, unsigned size) 
{
  memcpy(b, p, size); 
  p += size;
}

