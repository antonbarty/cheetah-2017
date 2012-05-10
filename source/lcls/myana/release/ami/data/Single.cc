#include "Single.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryTH2F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryFactory.hh"
#include "ami/data/FeatureExpression.hh"

#include <stdio.h>

using namespace Ami;

Single::Single(const char* scale) : 
  AbsOperator(AbsOperator::Single),
  _entry     (0),
  _term      (0)
{
  if (scale)
    strncpy(_scale_buffer,scale,SCALE_LEN);
  else
    memset(_scale_buffer,0,SCALE_LEN);
}

Single::Single(const char*& p, const DescEntry& e, FeatureCache& features) :
  AbsOperator(AbsOperator::Single)
{
  _extract(p,_scale_buffer, SCALE_LEN);

  _entry = EntryFactory::entry(e);
  _entry->desc().aggregate(false);

  if (_scale_buffer[0]) {
    QString expr(_scale_buffer);
    FeatureExpression parser;
    _term = parser.evaluate(features,expr);
    if (!_term)
      printf("BinMath failed to parse f %s\n",qPrintable(expr));
  }
  else
    _term = 0;
}

Single::~Single()
{
  if (_entry) delete _entry;
  if (_term ) delete _term;
}

DescEntry& Single::output   () const { return _entry->desc(); }

void*      Single::_serialize(void* p) const
{
  _insert(p, _scale_buffer , SCALE_LEN);
  return p;
}

#define SET_CASE(type) { \
  case DescEntry::type: \
    { Entry##type& entry = *reinterpret_cast<Entry##type*>(_entry); \
      entry.setto(reinterpret_cast<const Entry##type&>(e));         \
      entry.info (entry.info(Entry##type::Normalization)*v, Entry##type::Normalization); \
      break; } }

Entry&     Single::_operate(const Entry& e) const
{
  double v = _term ? _term->evaluate() : 1;

  switch(e.desc().type()) {
    SET_CASE(TH1F);
    SET_CASE(TH2F);
    SET_CASE(Prof);
    SET_CASE(Image);
    SET_CASE(Waveform);
  default:
    break;
  }
  return *_entry;
}

#undef SET_CASE
