#include "EnvPlot.hh"

#include "ami/data/FeatureCache.hh"
#include "ami/data/DescEntryW.hh"
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

using namespace Ami;


EnvPlot::EnvPlot(const DescEntry& output) :
  AbsOperator(AbsOperator::EnvPlot),
  _cache     (0),
  _term      (0),
  _weight    (0),
  _entry     (0),
  _input     (0)
{
  memcpy (_desc_buffer, &output, output.size());
}

EnvPlot::EnvPlot(const char*& p, FeatureCache& features, const Cds& cds) :
  AbsOperator(AbsOperator::EnvPlot),
  _cache (&features),
  _term  (0),
  _weight(0)
{
  _extract(p, _desc_buffer, DESC_LEN);

  const DescEntry& o = *reinterpret_cast<const DescEntry*>(_desc_buffer);

  _entry = EntryFactory::entry(o);

  FeatureExpression parser;
  { QString expr(o.name());
    _input = parser.evaluate(features,expr);
    if (!_input)
      printf("EnvPlot failed to parse %s\n",qPrintable(expr));
  }

  if (o.type()==DescEntry::Prof ||
      o.type()==DescEntry::Scan) {
    QString expr(o.xtitle());
    _term = parser.evaluate(features,expr);
    if (!_term)
      printf("EnvPlot failed to parse %s\n",qPrintable(expr));
  }

  if (o.isweighted_type()) {
    const DescEntryW& w = static_cast<const DescEntryW&>(o);
    if (w.weighted()) {
      QString expr(w.weight());
      printf("%s evaluates to\n",qPrintable(expr));
      _weight = parser.evaluate(features,expr);
      if (!_weight)
	printf("EnvPlot failed to parse %s\n",qPrintable(expr));
    }
  }
}

EnvPlot::~EnvPlot()
{
  if (_input ) delete _input;
  if (_term  ) delete _term;
  if (_weight) delete _weight;
  if (_entry ) delete _entry;
}

DescEntry& EnvPlot::output   () const 
{ 
  return _entry ? _entry->desc() : *reinterpret_cast<DescEntry*>(const_cast<char*>(_desc_buffer)); 
}

void*      EnvPlot::_serialize(void* p) const
{
  _insert(p, _desc_buffer, DESC_LEN);
  return p;
}

#include "pdsdata/xtc/ClockTime.hh"

Entry&     EnvPlot::_operate(const Entry& e) const
{
  if (_input != 0 && e.valid()) {
    Feature::damage(false);
    double y = _input->evaluate();
    double w = _weight ? _weight->evaluate() : 1;
    if (!Feature::damage()) {
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
	if (_term) {
	  bool damaged=false; double x=_term->evaluate();
	  if (!damaged) {
	    EntryProf* en = static_cast<EntryProf*>(_entry);
	    en->addy(y,x);
	    en->addinfo(1.,EntryProf::Normalization);
	  }
	}
	break;
      case DescEntry::Scan:    
	if (_term) {
	  bool damaged=false; double x=_term->evaluate();
	  if (!damaged) {
	    EntryScan* en = static_cast<EntryScan*>(_entry);
	    en->addy(y,x,w);
	    en->addinfo(1.,EntryScan::Normalization);
	  }
	} 
	break;
      case DescEntry::Waveform:
      case DescEntry::TH2F:
      case DescEntry::Image:
      default:
	printf("EnvPlot::_operator no implementation for type %d\n",_entry->desc().type());
	break;
      }
      _entry->valid(e.time());
    }
  }
  return *_entry;
}
