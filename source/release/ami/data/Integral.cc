#include "Integral.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryScalar.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryTH2F.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryFactory.hh"

#include <stdio.h>

using namespace Ami;

Integral::Integral(const DescEntry& d, unsigned xlo, unsigned xhi, unsigned ylo, unsigned yhi) :
  AbsOperator(AbsOperator::Integral),
  _entry     (EntryFactory::entry(d)),
  _xlo       (xlo),
  _xhi       (xhi),
  _ylo       (ylo),
  _yhi       (yhi)
{
}

Integral::Integral(const char*& p, const DescEntry& e) :
  AbsOperator(AbsOperator::Integral)
{
  const DescEntry& d = *reinterpret_cast<const DescEntry*>(p); 
  p += d.size();
  _entry = EntryFactory::entry(d);
  _extract(p, &_xlo, sizeof(_xlo));
  _extract(p, &_xhi, sizeof(_xhi));
  _extract(p, &_ylo, sizeof(_ylo));
  _extract(p, &_yhi, sizeof(_yhi));
}

Integral::~Integral()
{
  delete _entry;
}

DescEntry& Integral::output   () const { return _entry->desc(); }

void*      Integral::_serialize(void* p) const
{
  _insert(p, &_entry->desc(), _entry->desc().size());
  _insert(p, &_xlo, sizeof(_xlo));
  _insert(p, &_xhi, sizeof(_xhi));
  _insert(p, &_ylo, sizeof(_ylo));
  _insert(p, &_yhi, sizeof(_yhi));
  return p;
}

Entry&     Integral::_operate(const Entry& e) const
{
  if (!e.valid())
    return *_entry;

  double sum = 0;
  double n   = 0;
  switch(e.desc().type()) {
  case DescEntry::TH1F:
    { const EntryTH1F& en = static_cast<const EntryTH1F&>(e);
      for(unsigned k=_xlo; k<_xhi; k++)
	sum += en.content(k);
      n = en.info(EntryTH1F::Normalization);
      break; }
  case DescEntry::TH2F:
    { const EntryTH2F& en = static_cast<const EntryTH2F&>(e);
      for(unsigned k=_xlo; k<_xhi; k++)
	for(unsigned j=_ylo; j<_yhi; j++)
	  sum += en.content(k,j);
      n = en.info(EntryTH2F::Normalization);
      break; }
  case DescEntry::Prof:
    printf("Integrating Prof not implemented\n");
    break;
  case DescEntry::Image:
    { const EntryImage& en = static_cast<const EntryImage&>(e);
      const DescImage& d = en.desc();
      if (d.nframes())
	for(unsigned fn=0; fn<d.nframes(); fn++) {
	  int xlo(_xlo), xhi(_xhi), ylo(_ylo), yhi(_yhi);
	  if (d.xy_bounds(xlo,xhi,ylo,yhi,fn)) {
	    for(unsigned k=xlo; k<_xhi; k++)
	      for(unsigned j=ylo; j<_yhi; j++)
		sum += en.content(k,j);
	  }
	}
      else
	for(unsigned k=_xlo; k<_xhi; k++)
	  for(unsigned j=_ylo; j<_yhi; j++)
	    sum += en.content(k,j);
      n = en.info(EntryImage::Normalization);
      break; }
  case DescEntry::Waveform:
    { const EntryWaveform& en = static_cast<const EntryWaveform&>(e);
      for(unsigned k=_xlo; k<_xhi; k++)
	sum += en.content(k);
      n = en.info(EntryWaveform::Normalization);
      break; }
  default:
    printf("Integral: unknown input type %d\n",e.desc().type());
    break;
  }
  
  switch(_entry->desc().type()) {
  case DescEntry::Scalar:  
    { EntryScalar* en = static_cast<EntryScalar*>(_entry);
      en->addcontent(sum); 
      break; }
  case DescEntry::TH1F:    
    { EntryTH1F* en = static_cast<EntryTH1F*>(_entry);
      en->addcontent(1.,sum); 
      en->addinfo(n,EntryTH1F::Normalization);
      break; }
  default: break;
  }

  _entry->valid(e.time());
  return *_entry;
}
