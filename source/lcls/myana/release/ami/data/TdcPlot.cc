#include "TdcPlot.hh"

#include "ami/data/FeatureCache.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/EntryRef.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryTH2F.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/data/Cds.hh"
#include "ami/data/FeatureExpression.hh"

#include "pds/config/AcqDataType.hh"

#include <QtCore/QString>
#include <QtCore/QRegExp>

#include <stdio.h>

using namespace Ami;

static const double TC890_Period = 50e-12;

class TdcPlot::TdcVar : public Ami::Variable {
public:
  TdcVar(unsigned i) : _name(QString("{%1}").arg(i)) {}
  TdcVar(const TdcVar& c) : _name(c._name) {}
public:
  double evaluate() const { return _v; }
  Variable* clone() const { return new TdcVar(*this); }
  const QString& name() const { return _name; }
public:
  void set(double v) { _v=v; }
private:
  QString _name;
  double  _v;
};

TdcPlot::TdcPlot(const DescEntry& output, const char* expr) :
  AbsOperator(AbsOperator::TdcPlot),
  _xterm     (0),
  _output    (0)
{
  strncpy(_expression , expr, EXPRESSION_LEN);
  memcpy (_desc_buffer, &output, output.size());
}

TdcPlot::TdcPlot(const char*& p, const DescEntry& input) :
  AbsOperator(AbsOperator::TdcPlot),
  _mask      (0)
{
  _extract(p, _expression , EXPRESSION_LEN);
  _extract(p, _desc_buffer, DESC_LEN);

  const DescEntry& o = *reinterpret_cast<const DescEntry*>(_desc_buffer);

  _output = EntryFactory::entry(o);

  //  Parse the name to get the channel math
  for(unsigned i=0; i<6; i++)
    _chan[i] = new TdcVar(i);

  printf("TdcPlot parsing %s\n",_expression);

  QString expr(_expression);
  QString new_expr;
  // parse expression for channel indices
  QRegExp match("\\{[0-9]+\\}");
  int last=0;
  int pos=0;
  int mlen=0;
  while( (pos=match.indexIn(expr,pos)) != -1) {
    mlen = match.matchedLength();
    QString use = expr.mid(pos+1,mlen-2);
    unsigned ch = use.toInt();
    Term* t = _chan[ch];
    _mask |= (1<<ch);
    new_expr.append(expr.mid(last,pos-last));
    new_expr.append(QString("[%1]").arg((ulong)t,0,16));
    pos += mlen;
    last = pos;
  }
  new_expr.append(expr.mid(last));

  std::list<Variable*> vars;  // empty
  Expression parser(vars);
  
  if ((pos = new_expr.indexOf('%'))!=-1) {
    _yterm = parser.evaluate(new_expr.mid(0,pos));
    _xterm = parser.evaluate(new_expr.mid(pos+1));
  }
  else {
    _xterm = parser.evaluate(new_expr);
    _yterm = 0;
  }
  if (!_xterm)
    printf("TdcPlot failed to parse %s\n",qPrintable(new_expr));
}

TdcPlot::TdcPlot(const char*& p) :
  AbsOperator(AbsOperator::TdcPlot),
  _mask      (0),
  _xterm     (0),
  _output    (0)
{
  _extract(p, _expression , EXPRESSION_LEN);
  _extract(p, _desc_buffer, DESC_LEN);
}

TdcPlot::~TdcPlot()
{
  if (_xterm   ) {
    delete _xterm;
    for(unsigned i=0; i<6; i++)
      delete _chan[i];
    if (_yterm) 
      delete _yterm;
  }
  if (_output ) {
    delete _output;
  }
}

DescEntry& TdcPlot::output   () const 
{ 
  return _output ? _output->desc() : *reinterpret_cast<DescEntry*>(const_cast<char*>(_desc_buffer)); 
}

void*      TdcPlot::_serialize(void* p) const
{
  _insert(p, _expression , EXPRESSION_LEN);
  _insert(p, _desc_buffer, DESC_LEN);
  return p;
}

#include "pdsdata/xtc/ClockTime.hh"

typedef AcqTdcDataType::Marker  MarkerType;
typedef AcqTdcDataType::Channel ChannelType;

Entry&     TdcPlot::_operate(const Entry& e) const
{
  if (!e.valid())
    return *_output;

  if (!_xterm)
    return *_output;

  const EntryRef* input  = static_cast<const EntryRef*>(&e);

  if (output().type() != DescEntry::TH2F)
    _output->reset();

  //
  //  Depends upon the list of hits terminating in a Bank switch marker
  //
  unsigned mask(0);
  const AcqTdcDataType* p = reinterpret_cast<const AcqTdcDataType*>(input->data());
  while(!(p->source() == AcqTdcDataType::AuxIO &&
          static_cast<const MarkerType*>(p)->type() < MarkerType::AuxIOMarker)) {
    if (p->source()>AcqTdcDataType::Comm && 
        p->source()<AcqTdcDataType::AuxIO) {
      const ChannelType& c = *static_cast<const ChannelType*>(p);
      unsigned ch = p->source()-1;
      //  If the hit is valid and is one we're interested in
      if (!c.overflow() && (_mask&(1<<ch))) {
        double t = c.ticks()*TC890_Period;
        _chan[ch]->set(t);
        //  If we've seen at least one of each hit we need
        if ((mask |= 1<<ch)==_mask) {
          switch(output().type()) {
          case DescEntry::TH1F:
            static_cast<EntryTH1F*>(_output)->addcontent(1.,_xterm->evaluate());
            break;
          case DescEntry::TH2F:
            if (_yterm)
              static_cast<EntryTH2F*>(_output)->addcontent(1.,_xterm->evaluate(),_yterm->evaluate());
            break;
          case DescEntry::Image:
            if (_yterm) {
              EntryImage* o = static_cast<EntryImage*>(_output);
              double x = _xterm->evaluate();
              if (x>=0 && x<o->desc().nbinsx()) {
                double y = _yterm->evaluate();
                if (y>=0 && y<o->desc().nbinsy())
                  o->addcontent(1,unsigned(x),o->desc().nbinsy()-unsigned(y)-1);
              }
            }
            break;
          default:
            break;
          }
        }
      }
    }
    p++;
  }

  switch(output().type()) {
  case DescEntry::TH1F:
    static_cast<EntryTH1F* >(_output)->addinfo(1,EntryTH1F ::Normalization); break;
  case DescEntry::TH2F:
    static_cast<EntryTH2F* >(_output)->addinfo(1,EntryTH2F ::Normalization); break;
  case DescEntry::Image:
    static_cast<EntryImage*>(_output)->addinfo(1,EntryImage::Normalization); break;
  default:
    break;
  }

  _output->valid(e.time());
  return *_output;
}
