#include "EntryMath.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryFactory.hh"
#include "ami/data/EntryTerm.hh"
#include "ami/data/FeatureCache.hh"

#include "ami/data/Cds.hh"
#include "ami/data/Expression.hh"
#include "ami/data/FeatureExpression.hh"

#include "pdsdata/xtc/ClockTime.hh"

#include <QtCore/QString>
#include <QtCore/QRegExp>

#include <stdio.h>

using namespace Ami;

EntryMath::EntryMath(const char* expr) : 
  AbsOperator(AbsOperator::EntryMath),
  _term      (0),
  _entry     (0)
{
  strncpy(_expression, expr, EXPRESSION_LEN);
}

#define CASETERM(type) case DescEntry::type: \
  t = new Entry##type##Term(static_cast<const Entry##type&>(*entry),_index); break;

EntryMath::EntryMath(const char*& p, const DescEntry& e, const Cds& cds, FeatureCache& features) :
  AbsOperator(AbsOperator::EntryMath)
{
  _extract(p, _expression, EXPRESSION_LEN);
  _entry = EntryFactory::entry(e);

  // parse expression for signatures
  QRegExp match("\\[[0-9]+\\]");
  QString expr(_expression);
  QString new_expr;
  int last=0;
  int pos=0;
  while( (pos=match.indexIn(expr,pos)) != -1) {
    QString use = expr.mid(pos+1,match.matchedLength()-2);
    const Entry* entry = cds.entry(use.toInt());
    if (!entry) {
      printf("EntryMath: Unable to lookup signature %d\n",use.toInt());
      exit(1);
    }
    Term* t;
    switch(entry->desc().type()) {
      CASETERM(Waveform);
      CASETERM(TH1F);
      CASETERM(Prof);
      CASETERM(Image);
    default:
      printf("EntryMath: No implementation for entry type %d\n",entry->desc().type());
      t = 0;
      break;
    }
    new_expr.append(expr.mid(last,pos-last));
    new_expr.append(QString("[%1]").arg((ulong)t,0,16));
    pos += match.matchedLength();
    last = pos;
  }
  new_expr.append(expr.mid(last));

#if 0
  std::list<Variable*> variables; // none
  Expression parser(variables);
  _term = parser.evaluate(new_expr);
#else
  FeatureExpression parser;
  _term = parser.evaluate(features,new_expr);
#endif
}

EntryMath::~EntryMath()
{
  if (_term) delete _term;
  if (_entry) delete _entry;
}

DescEntry& EntryMath::output   () const { return _entry->desc(); }

void*      EntryMath::_serialize(void* p) const
{
  _insert(p, _expression, EXPRESSION_LEN);
  return p;
}

#define CASE_1D(type,func) case DescEntry::type:			\
  { Entry##type& en = static_cast<Entry##type&>(*_entry);		\
    for(_index=0; _index<en.desc().nbins(); _index++)			\
      en.func( _term->evaluate(), _index );				\
    break;								\
  }

#define CASE_2D(type) case DescEntry::type:				\
  { Entry##type& en = static_cast<Entry##type&>(*_entry);		\
    _index = 0;								\
    for(unsigned j=0; j<en.desc().nbinsy(); j++)			\
      for(unsigned k=0; k<en.desc().nbinsx(); k++) {			\
	double v = _term->evaluate() + en.info(Entry##type::Pedestal);	\
	unsigned iv = (v>0) ? unsigned(v) : 0;				\
	en.contents()[_index++] = iv;					\
      }									\
    break;								\
  }

Entry&     EntryMath::_operate(const Entry& e) const
{
  _entry->valid(e.time());
  if (!e.valid())
    return *_entry;

  switch(e.desc().type()) {
    CASE_1D(TH1F    ,content);
    CASE_1D(Waveform,content);
    CASE_1D(Prof    ,addy);
    CASE_2D(Image);
  default:
    printf("EntryMath::_operator no implementation for type %d\n",e.desc().type());
    break;
  }
  return *_entry;
}
