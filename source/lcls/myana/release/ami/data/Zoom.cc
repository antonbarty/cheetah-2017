#include "Zoom.hh"

#include "ami/data/DescImage.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryFactory.hh"

#include <stdio.h>

using namespace Ami;

Zoom::Zoom(const DescEntry& d, const AbsOperator& o) :
  AbsOperator(AbsOperator::Zoom),
  _entry     (static_cast<EntryImage*>(EntryFactory::entry(d)))
{
  //  Append an operator to this one
  //  Assumes the operator works on images of any size
  if (o.type()==AbsOperator::Average)
    next(const_cast<AbsOperator*>(&o));
}

Zoom::Zoom(const char*& p, const DescEntry& e) :
  AbsOperator(AbsOperator::Zoom)
{
  const DescEntry& d = *reinterpret_cast<const DescEntry*>(p); 
  p += d.size();
  _entry = static_cast<EntryImage*>(EntryFactory::entry(d));
}

Zoom::~Zoom()
{
  delete _entry;
}

DescEntry& Zoom::output   () const { return _entry->desc(); }

void*      Zoom::_serialize(void* p) const
{
  _insert(p, &_entry->desc(), _entry->desc().size());
  return p;
}

Entry&     Zoom::_operate(const Entry& e) const
{
  if (!e.valid())
    return *_entry;

  switch(e.desc().type()) {
  case DescEntry::Image:
    { const EntryImage& en = static_cast<const EntryImage&>(e);
      const DescImage& d = en.desc();
      const DescImage& desc = _entry->desc();
      unsigned* p = _entry->contents();
      const unsigned* q = en.contents();
      q += unsigned(desc.ylow())*d.nbinsx();
      q += unsigned(desc.xlow());
      for(unsigned j=0; j<desc.nbinsy(); j++, q+=d.nbinsx()) {
        const unsigned* qr = q;
        for(unsigned k=0; k<desc.nbinsx(); k++)
          *p++ = *qr++;
      }
      float* f = reinterpret_cast<float*>(p);
      for(unsigned j=0; j<EntryImage::InfoSize; j++)
        *f++ = en.info(EntryImage::Info(j));
      break; }
  default:
    printf("Zoom: unknown input type %d\n",e.desc().type());
    break;
  }
  
  _entry->valid(e.time());
  return *_entry;
}
