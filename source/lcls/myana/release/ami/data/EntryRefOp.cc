#include "EntryRefOp.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryRef.hh"

#include <sys/uio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace Ami;

EntryRefOp::EntryRefOp(unsigned index) : 
  AbsOperator(AbsOperator::EntryRefOp),
  _index     (index),
  _output    (0)
{
}

EntryRefOp::EntryRefOp(const char*& p, const DescEntry& e) :
  AbsOperator(AbsOperator::EntryRefOp)
{
  _extract(p, &_index, sizeof(_index));
  
  void* data;
  sscanf(e.ytitle(),"%p",&data);

  Entry** ptr = reinterpret_cast<Entry**>(data);
  _output = &ptr[_index]->desc();
}

EntryRefOp::~EntryRefOp()
{
  if (next()) delete next();
}

static DescRef _no_entry("NoEntry","none");

DescEntry& EntryRefOp::output   () const 
{ 
  return *_output;
}

void*      EntryRefOp::_serialize(void* p) const
{
  _insert(p, &_index, sizeof(_index));
  return p;
}

Entry&     EntryRefOp::_operate(const Entry& e) const
{
  const EntryRef& ref = static_cast<const EntryRef&>(e);
  //  const cast is OK because this operator's output is always
  //  just input to another operator.
  Entry** p = reinterpret_cast<Entry**>(const_cast<void*>(ref.data()));
  return *(p[_index]);
}
