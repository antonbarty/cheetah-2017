#include "PeakFitPlot.hh"

#include "ami/data/FeatureCache.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryScalar.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryScan.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/data/Cds.hh"
#include "ami/data/FeatureExpression.hh"

#include <QtCore/QString>

#include <stdio.h>

namespace Ami {
  class EntryAccessor {
  public:
    virtual ~EntryAccessor() {}
    virtual double bin_value(unsigned,bool&) const = 0;
    virtual unsigned nbins() const = 0;
    virtual double xlow() const = 0;
    virtual double xup () const = 0;
  };
  class TH1FAccessor : public EntryAccessor {
  public:
    TH1FAccessor(const EntryTH1F& e) : _entry(e) {}
    double bin_value(unsigned bin,
		     bool& valid) const 
    { valid=true; return _entry.content(bin); }
    unsigned nbins() const { return _entry.desc().nbins(); }
    double xlow() const { return _entry.desc().xlow(); }
    double xup () const { return _entry.desc().xup (); }
  private:
    const EntryTH1F& _entry;
  };
  class ProfAccessor : public EntryAccessor {
  public:
    ProfAccessor(const EntryProf& e) : _entry(e) {}
    double bin_value(unsigned bin,
		     bool& valid) const 
    {
      double n=_entry.nentries(bin);
      if (!(n>0)) { valid=false; return 0; }
      else { valid = true; return _entry.ysum(bin)/n; } 
    }
    unsigned nbins() const { return _entry.desc().nbins(); }
    double xlow() const { return _entry.desc().xlow(); }
    double xup () const { return _entry.desc().xup (); }
  private:
    const EntryProf& _entry;
  };
};

using namespace Ami;


PeakFitPlot::PeakFitPlot(const DescEntry& output, 
			 double    baseline,
			 Parameter prm) :
  AbsOperator(AbsOperator::PeakFitPlot),
  _baseline  (baseline),
  _prm       (prm),
  _cache     (0),
  _term      (0),
  _entry     (0)
{
  memcpy (_desc_buffer, &output, output.size());
}

PeakFitPlot::PeakFitPlot(const char*& p, FeatureCache& features) :
  AbsOperator(AbsOperator::PeakFitPlot),
  _cache (&features),
  _term  (0)
{
  _extract(p, _desc_buffer, DESC_LEN);
  _extract(p, &_baseline  , sizeof(_baseline));
  _extract(p, &_prm  , sizeof(_prm  ));

  const DescEntry& o = *reinterpret_cast<const DescEntry*>(_desc_buffer);

  _entry = EntryFactory::entry(o);

  if (o.type()==DescEntry::Prof ||
      o.type()==DescEntry::Scan) {
    QString expr(o.xtitle());
    FeatureExpression parser;
    _term = parser.evaluate(features,expr);
    if (!_term)
      printf("PeakFitPlot failed to parse %s\n",qPrintable(expr));
  }
}

PeakFitPlot::PeakFitPlot(const char*& p) :
  AbsOperator(AbsOperator::PeakFitPlot),
  _cache(0),
  _term (0),
  _entry(0)
{
  _extract(p, _desc_buffer, DESC_LEN);
  _extract(p, &_baseline  , sizeof(_baseline));
  _extract(p, &_prm  , sizeof(_prm  ));
}

PeakFitPlot::~PeakFitPlot()
{
  if (_term ) delete _term;
  if (_entry) delete _entry;
}

PeakFitPlot::Parameter  PeakFitPlot::prm      () const { return _prm; }

DescEntry& PeakFitPlot::output   () const 
{ 
  return _entry ? _entry->desc() : *reinterpret_cast<DescEntry*>(const_cast<char*>(_desc_buffer)); 
}

void*      PeakFitPlot::_serialize(void* p) const
{
  _insert(p, _desc_buffer, DESC_LEN);
  _insert(p, &_baseline  , sizeof(_baseline));
  _insert(p, &_prm  , sizeof(_prm));
  return p;
}

Entry&     PeakFitPlot::_operate(const Entry& e) const
{
  if (!e.valid())
    return *_entry;

  EntryAccessor* acc;
  switch(e.desc().type()) {
  case DescEntry::TH1F: acc = new TH1FAccessor(static_cast<const EntryTH1F&>(e)); break;
  case DescEntry::Prof: acc = new ProfAccessor(static_cast<const EntryProf&>(e)); break;
  default: 
    printf("PeakFit on type %d not implemented\n",e.desc().type());
    return *_entry;
  }

  double y=0;
  const unsigned nbins = acc->nbins();
  if (_prm==RMS) {
    double x0=0, x1=0, x2=0;
    for(unsigned i=0; i<nbins; i++) {
      bool lv;
      double v  = acc->bin_value(i,lv)-_baseline;
      if (v>0 && lv) {
	double vi = v*double(i);
	x0 += v;
	x1 += vi;
	x2 += vi*double(i);
      }
    }
    if (x0>0) {
      x1 /= x0;
      x2 /= x0;
      y = sqrt(x2-x1*x1)*(acc->xup()-acc->xlow())/double(nbins);
    }
    else
      return *_entry;
  }
  else {
    //
    //  Find the peak
    //
    bool valid;
    unsigned v=0; y=acc->bin_value(0,valid);
    for(unsigned i=1; i<nbins; i++) {
      bool lv;
      double z = acc->bin_value(i,lv);
      if (lv) {
	if (!valid) { y=z; v=i; valid=true; }
	else if (z>y) { y=z; v=i; }
      }
    }
    if (!valid)
      return *_entry;

    if (_prm==FWHM) {  // Find closest edges that fall below half maximum

      double y2 = 0.5*(y+_baseline);
      if (y2 < 0)
	return *_entry;

      //
      //  iterate left of the peak 
      //    
      double x0=0;      // location of half-maximum left of the peak
      { double   x =y;  // value of last valid point
	unsigned xi=v;  // index of last valid point
	unsigned i=v-1; // test index
	while(i>0) {
	  bool lv;
	  double z = acc->bin_value(i,lv);
	  if (lv) {
	    if (z < y2) {  // found it. interpolate
	      x0 = (double(i)*(x-y2) + double(xi)*(y2-z)) / (x-z);
	      break;
	    }
	    x  = z;
	    xi = i;
	  }
	  i--;
	}
      }

      //
      //  iterate right of the peak
      //
      double x1=nbins;  // location of half-maximum right of the peak
      { double   x =y;         // value of last valid point
	unsigned xi=v;         // index of last valid point 
	unsigned i=v+1;        // test index
	while(i<nbins) {
	  bool lv;
	  double z = acc->bin_value(i,lv);
	  if (lv) {
	    if (z < y2) {  // found it. interpolate
	      x1 = (double(i)*(x-y2) + double(xi)*(y2-z)) / (x-z);
	      break;
	    }
	    x  = z;
	    xi = i;
	  }
	  i++;
	}
      }

      //  scale the width from bins into physical units      
      y = (x1-x0)*(acc->xup()-acc->xlow())/double(nbins);
    }

    else {  // quadratic fit

      if (v == 0) {
	if (_prm == Position)
	  y = acc->xlow();
	else
	  return *_entry;
      }
      else if (v >= nbins-1) {
	if (_prm == Position)
	  y = acc->xup();
	else
	  return *_entry;
      }
      else {
	//
	//  Find the first valid points left and right of the peak
	//
	unsigned il=v-1; // test index
	double yl;
	while(il>0) {
	  bool lv;
	  yl = acc->bin_value(il,lv);
	  if (lv) 
	    break;
	  il--;
	}
	if (il == 0)
	  return *_entry;

	unsigned ir=v+1; // test index
	double yr;
	while(ir<nbins) {
	  bool lv;
	  yr = acc->bin_value(ir,lv);
	  if (lv) 
	    break;
	  ir++;
	}
	if (ir >= nbins)
	  return *_entry;

	double di  = double(ir-il);
	double di2 = double(ir*ir-il*il);
	double si  = double(v)  -0.5*double(ir+il);
	double si2 = double(v*v)-0.5*double(ir*ir+il*il);
	double dy  = yr   -yl;
	double sy  = y  -0.5*(yr+yl);

	double i0 = 0.5*(si2*dy - di2*sy) / (si*dy-di*sy);
	if (_prm == Position)
	  y = acc->xlow() + i0*(acc->xup()-acc->xlow())/double(nbins);
	else {  // Height
	  y -= dy*pow(double(v)-i0,2) / (di2 - 2*i0*di);
	}
      }
      if (_prm == Height)
	y -= _baseline;
    }
  }

  bool damaged=false;
  switch(_entry->desc().type()) {
  case DescEntry::Scalar: 
    { EntryScalar* en = static_cast<EntryScalar*>(_entry);
      en->addcontent(y);    
      break; }
  case DescEntry::TH1F: 
    { EntryTH1F* en = static_cast<EntryTH1F*>(_entry);
      en->addcontent(1.,y); 
      en->addinfo(1.,EntryTH1F::Normalization);
      break; }
  case DescEntry::Prof:    
    if (!_term)
      return *_entry;
    { double x=_term->evaluate();
      if (!damaged) {
	EntryProf* en = static_cast<EntryProf*>(_entry);
	en->addy(y,x);
	en->addinfo(1.,EntryProf::Normalization);
      }
      break; }
  case DescEntry::Scan:    
    if (!_term)
      return *_entry;
    { double x=_term->evaluate();
      if (!damaged) {
	EntryScan* en = static_cast<EntryScan*>(_entry);
	en->addy(y,x);
	en->addinfo(1.,EntryScan::Normalization);
      }
      break; }
  case DescEntry::Waveform:
  case DescEntry::TH2F:
  case DescEntry::Image:
  default:
    printf("PeakFitPlot::_operator no implementation for type %d\n",_entry->desc().type());
    break;
  }
  if (!damaged)
    _entry->valid(e.time());
  return *_entry;
}

const char* PeakFitPlot::name(Parameter p)
{
  static const char* names[] = {"Position", "Height", "FWHM", "RMS", NULL };
  return names[p];
}
