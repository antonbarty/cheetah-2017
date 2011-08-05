#include "PeakFinder.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/DescImage.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/data/Cds.hh"
#include "ami/data/Expression.hh"

#include <stdio.h>

using namespace Ami;

PeakFinder::PeakFinder(unsigned   threshold_value) :
  AbsOperator(AbsOperator::PeakFinder),
  _threshold_value(threshold_value),
  _output_entry(0)
{
}

#define CASETERM(type) case DescEntry::type: \
  t = new Entry##type##Term(static_cast<const Entry##type&>(*entry),_index); break;

static const char* _advance(const char*& p, unsigned size) { const char* o=p; p+=size; return o; }
#define EXTRACT(p, type) *(reinterpret_cast<const type*>(_advance(p,sizeof(type))))

PeakFinder::PeakFinder(const char*& p, const DescEntry& e) :
  AbsOperator     (AbsOperator::PeakFinder),
  _threshold_value(EXTRACT(p, unsigned)),
  _output_entry   (static_cast<EntryImage*>(EntryFactory::entry(e)))
{
  _output_entry->info(0,EntryImage::Pedestal);
  _threshold_value *= _output_entry->desc().ppxbin();
  _threshold_value *= _output_entry->desc().ppybin();
}

PeakFinder::~PeakFinder()
{
  if (_output_entry)
    delete _output_entry;
}

DescEntry& PeakFinder::output   () const { return _output_entry->desc(); }

void*      PeakFinder::_serialize(void* p) const
{
  _insert(p, &_threshold_value, sizeof(_threshold_value));
  return p;
}

Entry&     PeakFinder::_operate(const Entry& e) const
{
  if (!e.valid()) 
    return *_output_entry;

  const EntryImage& entry = static_cast<const EntryImage&>(e);
  const DescImage& d = entry.desc();
  const unsigned nx = d.nbinsx();
  const unsigned ny = d.nbinsy();
  const unsigned threshold = _threshold_value + entry.info(EntryImage::Pedestal);

  // find the peak positions which are above the threshold
  const unsigned* a = entry.contents();
  for(unsigned k=1; k<ny-1; k++, a+=nx) {
    const unsigned* b = a + nx;
    const unsigned* c = b + nx;
    for(unsigned j=1; j<nx-1; j++) {
      unsigned v = b[j];
      if (v > threshold &&
	  v > b[j-1] && 
	  v > b[j+1] &&
	  v > a[j-1] && 
	  v > a[j+0] &&
	  v > a[j+1] &&
	  v > c[j-1] && 
	  v > c[j+0] &&
	  v > c[j+1])
	_output_entry->addcontent(1,j,k);
    }
  }
  _output_entry->valid(e.time());
  return *_output_entry;
}
