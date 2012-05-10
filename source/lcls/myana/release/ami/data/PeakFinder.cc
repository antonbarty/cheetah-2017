#include "PeakFinder.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/DescImage.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryFactory.hh"
#include "ami/data/PeakFinderFn.hh"

#include "ami/data/Cds.hh"
#include "ami/data/Expression.hh"

#include <stdio.h>

using namespace Ami;

static PeakFinderFn* _lookup(unsigned);

PeakFinder::PeakFinder(double threshold_v0,
                       double threshold_v1,
                       bool   accumulate) :
  AbsOperator(AbsOperator::PeakFinder),
  _threshold_v0(threshold_v0),
  _threshold_v1(threshold_v1),
  _accumulate  (accumulate),
  _output_entry(0),
  _fn          (0)
{
  printf("PeakFinder acc %c\n",_accumulate ? 't':'f');
}

#define CASETERM(type) case DescEntry::type: \
  t = new Entry##type##Term(static_cast<const Entry##type&>(*entry),_index); break;

static const char* _advance(const char*& p, unsigned size) { const char* o=p; p+=size; return o; }
#define EXTRACT(p, type) *(reinterpret_cast<const type*>(_advance(p,sizeof(type))))

PeakFinder::PeakFinder(const char*& p, const DescEntry& e) :
  AbsOperator     (AbsOperator::PeakFinder),
  _threshold_v0(EXTRACT(p, double)),
  _threshold_v1(EXTRACT(p, double)),
  _accumulate  (EXTRACT(p, bool  )),
  _output_entry(static_cast<EntryImage*>(EntryFactory::entry(e))),
  _fn          (0)
{
  printf("PeakFinder2 acc %c\n",_accumulate ? 't':'f');

  _output_entry->info(0,EntryImage::Pedestal);
  int ppbin = 1;
  ppbin *= _output_entry->desc().ppxbin();
  ppbin *= _output_entry->desc().ppybin();
  double dppbin(ppbin);
  _threshold_v0 *= dppbin;
  _threshold_v1 *= dppbin;

  if ((_fn = _lookup(e.info().phy())))
    _fn->setup(_threshold_v0,
               _threshold_v1);
}

PeakFinder::~PeakFinder()
{
  if (_output_entry)
    delete _output_entry;
  if (_fn)
    delete _fn;
}

DescEntry& PeakFinder::output   () const { return _output_entry->desc(); }

void*      PeakFinder::_serialize(void* p) const
{
  _insert(p, &_threshold_v0, sizeof(_threshold_v0));
  _insert(p, &_threshold_v1, sizeof(_threshold_v1));
  _insert(p, &_accumulate  , sizeof(_accumulate));
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

  if (!_accumulate)
    memset(_output_entry->contents(), 0, sizeof(unsigned)*nx*ny);

  if (_fn) {
    // find the peak positions which are above the threshold
    const unsigned* a = entry.contents();
    for(unsigned k=0; k<ny; k++) {
      for(unsigned j=0; j<nx; j++) {
        const unsigned threshold = _fn->value(j,k) + unsigned(entry.info(EntryImage::Pedestal));
        unsigned v = *a++;
        if (v > threshold)
          _output_entry->addcontent(1,j,k);
      }
    }
  }
  else {
    // find the peak positions which are above the threshold
    const unsigned threshold = unsigned(_threshold_v0) + unsigned(entry.info(EntryImage::Pedestal));
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
    }  }

  _output_entry->valid(e.time());
  return *_output_entry;
}

#include <map>

static std::map<unsigned,PeakFinderFn*> _fn_map;

void PeakFinder::register_(unsigned      phy,
                           PeakFinderFn* fn)
{
  if (_fn_map[phy])
    delete _fn_map[phy];

  _fn_map[phy] = fn;
}

PeakFinderFn* _lookup(unsigned phy)
{
  std::map<unsigned,PeakFinderFn*>::iterator it = _fn_map.find(phy);
  return (it == _fn_map.end()) ? 0 : _fn_map[phy]->clone();
}
