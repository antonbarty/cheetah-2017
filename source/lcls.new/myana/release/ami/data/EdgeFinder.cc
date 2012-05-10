#include "EdgeFinder.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryTH2F.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/data/Cds.hh"
#include "ami/data/Expression.hh"

#include <stdio.h>

using namespace Ami;

EdgeFinder::EdgeFinder(double     fraction,
		       double     threshold_value,
		       double     baseline_value,
                       int        alg,
                       double     deadtime,
		       const      DescEntry& output,
                       Parameter  parameter) :
  AbsOperator(AbsOperator::EdgeFinder),
  _fraction (fraction),
  _alg(alg),
  _deadtime(deadtime),
  _threshold_value(threshold_value),
  _baseline_value(baseline_value),
  _output_entry(0),
  _parameter(parameter)
{
  char* b = new char[output.size()];
  memcpy(b,&output,output.size());
  _output = reinterpret_cast<DescEntry*>(b);
}

#define CASETERM(type) case DescEntry::type: \
  t = new Entry##type##Term(static_cast<const Entry##type&>(*entry),_index); break;

static const char* _advance(const char*& p, unsigned size) { const char* o=p; p+=size; return o; }
#define EXTRACT(p, type) *(reinterpret_cast<const type*>(_advance(p,sizeof(type))))

EdgeFinder::EdgeFinder(const char*& p) :
  AbsOperator     (AbsOperator::EdgeFinder),
  _fraction       (EXTRACT(p, double)),
  _alg            (EXTRACT(p, int)),
  _deadtime       (EXTRACT(p, double)),
  _threshold_value(EXTRACT(p, double)),
  _baseline_value (EXTRACT(p, double))
{
  const DescEntry& output = *reinterpret_cast<const DescEntry*>(p);
  char* b = new char[output.size()];
  memcpy(b,p,output.size());
  p += output.size();
  _output = reinterpret_cast<DescEntry*>(b);

  _output_entry   = EntryFactory::entry(*_output);

  _parameter      = Parameter(EXTRACT(p, int));
}

EdgeFinder::EdgeFinder(double fraction, int alg, double deadtime, const char*& p) :
  AbsOperator     (AbsOperator::EdgeFinder),
  _fraction       (fraction),
  _alg            (alg),
  _deadtime       (deadtime),
  _threshold_value(EXTRACT(p, double)),
  _baseline_value (EXTRACT(p, double))
{
  const DescEntry& output = *reinterpret_cast<const DescEntry*>(p);
  char* b = new char[output.size()];
  memcpy(b,p,output.size());
  p += output.size();
  _output = reinterpret_cast<DescEntry*>(b);

  _output_entry   = EntryFactory::entry(*_output);

  _parameter      = Parameter(EXTRACT(p, int));
}

EdgeFinder::~EdgeFinder()
{
  delete _output;
  if (_output_entry)
    delete _output_entry;
}

DescEntry& EdgeFinder::output   () const { return _output_entry->desc(); }
void* EdgeFinder::desc   () const { return (void *)_output; }
int EdgeFinder::desc_size   () const { return _output->size(); }

void*      EdgeFinder::_serialize(void* p) const
{
  _insert(p, &_fraction, sizeof(_fraction));
  _insert(p, &_alg, sizeof(_alg));
  _insert(p, &_deadtime, sizeof(_deadtime));
  _insert(p, &_threshold_value, sizeof(_threshold_value));
  _insert(p, &_baseline_value, sizeof(_baseline_value));
  _insert(p,  _output, _output->size());
  _insert(p, &_parameter, sizeof(_parameter));
  return p;
}

Entry&     EdgeFinder::_operate(const Entry& e) const
{
  if (!e.valid())
    return *_output_entry;

  const EntryWaveform& entry = static_cast<const EntryWaveform&>(e);
  const DescWaveform& d = entry.desc();
  // find the boundaries where the pulse crosses the threshold
  double sc = entry.info(EntryWaveform::Normalization);
  if (sc==0) sc=1;
  double threshold_value = _threshold_value*sc;
  double baseline_value  = _baseline_value*sc;

  double   peak = threshold_value;
  unsigned start  =0;
  double   last   = -1.0;
  bool     crossed=false;
  bool     rising = threshold_value > baseline_value;
  for(unsigned k=0; k<d.nbins(); k++) {
    double y = entry.content(k);
    bool over = 
      ( rising && y>threshold_value) ||
      (!rising && y<threshold_value);
    if (!crossed && over) {
      crossed = true;
      start   = k;
      peak    = y;
    }
    else if (crossed && !over) {
      //  find the edge
      double edge_v = _fraction*(peak+baseline_value);
      unsigned i=start;
      if (rising == IsLeading(_alg)) { // leading positive edge, or trailing negative edge
	while(entry.content(i) < edge_v)
	  i++;
      }
      else {                           // trailing positive edge, or leading negative edge
	while(entry.content(i) > edge_v)
	  i++;
      }
      double edge = i>0 ? 
	(edge_v-entry.content(i))/(entry.content(i)-entry.content(i-1)) 
	+ double(i) : 0;
      double thisx = edge*(d.xup()-d.xlow())/double(d.nbins())+d.xlow();
      if (last < 0 || thisx > last + _deadtime) {
        switch(_parameter) {
        case Location:
          static_cast<EntryTH1F*>(_output_entry)->addcontent(1.,thisx);
          break;
        case Amplitude:
          static_cast<EntryTH1F*>(_output_entry)->addcontent(1.,peak);
          break;
        default:
          if (_output->type()==DescEntry::TH2F)
            static_cast<EntryTH2F*>(_output_entry)->addcontent(1.,thisx,peak);
          else
            ;
          break;
        }
        last = thisx;
      }
      crossed = false;
    }
    else if (( rising && y>peak) ||
	     (!rising && y<peak)) {
      peak = y;
      if (!IsLeading(_alg))  // For a trailing edge, start at the peak!
        start = k;
    }
  }
  switch(_output->type()) {
  case DescEntry::TH1F:
    static_cast<EntryTH1F*>(_output_entry)->addinfo(entry.info(EntryWaveform::Normalization),
                                                    EntryTH1F::Normalization);
    break;
  case DescEntry::TH2F:
    static_cast<EntryTH2F*>(_output_entry)->addinfo(entry.info(EntryWaveform::Normalization),
                                                    EntryTH2F::Normalization);
    break;
  default:
    break;
  }
  _output_entry->valid(e.time());
  return *_output_entry;
}
