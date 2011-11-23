//
//  Completes the parsing of expressions involving bin indices.
//  Indices for 1-d objects appear as [bin#] or [bin#,bin#].
//  Indices for 2-d objects appear as [bin#][bin#] or [bin#,bin#][bin#,bin#]
//
#include "BinMath.hh"

#include "ami/data/FeatureCache.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryScalar.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryScan.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryFactory.hh"
#include "ami/data/DescImage.hh"

#include "ami/data/Cds.hh"
#include "ami/data/FeatureExpression.hh"

#include <QtCore/QString>
#include <QtCore/QChar>
#include <QtCore/QRegExp>

#include <stdio.h>

static bool _parseIndices(const QString& use, unsigned& lo, unsigned& hi);

static const unsigned MAX_INDEX = 999999;

//
//  this could probably be a template
//
#define CLASSTERM(type,func) \
  class Entry##type##Term : public Ami::Term {				\
  public:								\
    Entry##type##Term(const Entry*& e, unsigned lo, unsigned hi) :	\
      _entry(e), _lo(lo), _hi(hi) {}					\
      ~Entry##type##Term() {}						\
  public:								\
      double evaluate() const						\
      { double sum=0;							\
	unsigned lo=_lo, hi=_hi;					\
	const Entry##type* e = static_cast<const Entry##type*>(_entry); \
	for(unsigned i=lo; i<=hi; i++)					\
	  sum += e->func(i);						\
	double n = e->info(Entry##type::Normalization);			\
	return n > 0 ? sum / n : sum; }					\
  private:								\
      const Entry*& _entry;						\
      unsigned _lo, _hi;						\
  }

namespace Ami {
  namespace BinMathC {
    CLASSTERM(Waveform,content);
    CLASSTERM(TH1F    ,content);
    CLASSTERM(Prof    ,ymean  );

    class EntryImageTerm : public Ami::Term {
    public:
      EntryImageTerm(const Entry*& e, unsigned xlo, unsigned xhi, unsigned ylo, unsigned yhi) :
	_entry(e), _xlo(xlo), _xhi(xhi), _ylo(ylo), _yhi(yhi) {}
      ~EntryImageTerm() {}
    public:
      double evaluate() const {
	const EntryImage& e = *static_cast<const EntryImage*>(_entry);
	const DescImage&  d = e.desc();
	double sum = 0;
	double p   = double(e.info(EntryImage::Pedestal));
	if (d.nframes()) {
	  for(unsigned fn=0; fn<d.nframes(); fn++) {
	    int xlo(_xlo), xhi(_xhi+1), ylo(_ylo), yhi(_yhi+1);
	    if (d.xy_bounds(xlo, xhi, ylo, yhi, fn))
	      for(int j=ylo; j<yhi; j++)
		for(int i=xlo; i<xhi; i++)
		  sum += double(e.content(i,j))-p;
	  }
	}
	else {
	  unsigned xlo=_xlo, xhi=_xhi, ylo=_ylo, yhi=_yhi;
	  for(unsigned j=ylo; j<=yhi; j++)
	    for(unsigned i=xlo; i<=xhi; i++)
	      sum += double(e.content(i,j))-p;
	}
	double n = double(e.info(EntryImage::Normalization));
	return n > 1 ? sum / n : sum;
      }
    private:
      const Entry*& _entry;
      unsigned _xlo, _xhi, _ylo, _yhi;
    };

    class EntryImageTermF : public Ami::Term {
    public:
      EntryImageTermF(const Entry*& e, double xc, double yc, double r0, double r1, double f0, double f1) :
	_entry(e), _xc(xc), _yc(yc), _r0(r0), _r1(r1), _f0(f0), _f1(f1) {}
      ~EntryImageTermF() {}
    public:
      double evaluate() const {
	const EntryImage& e = *static_cast<const EntryImage*>(_entry);
	const DescImage& d = e.desc();
	double sum = 0;
	const double p = e.info(EntryImage::Pedestal);
	int ixlo, ixhi, iylo, iyhi;
	if (d.nframes()) {
	  for(unsigned fn=0; fn<d.nframes(); fn++)
	    if (d.rphi_bounds(ixlo, ixhi, iylo, iyhi,
			      _xc, _yc, _r1, fn)) {
	      double xc(_xc), yc(_yc);
	      double r0sq(_r0*_r0), r1sq(_r1*_r1);
	      for(int j=iylo; j<=iyhi; j++) {
		double dy  = d.biny(j)-yc;
		double dy2 = dy*dy;
		for(int i=ixlo; i<=ixhi; i++) {
		  double dx  = d.binx(i)-xc;
		  double dx2 = dx*dx;
		  double rsq = dx2 + dy2;
		  double f   = atan2(dy,dx);
		  if ( (rsq >= r0sq && rsq <= r1sq) &&
		       ( (f>=_f0 && f<=_f1) ||
			 (f+2*M_PI <= _f1) ) )
		    sum += double(e.content(i,j))-p;
		}
	      }
	    }
          else
	      return 0;
	}
	else {
	  if (d.rphi_bounds(ixlo, ixhi, iylo, iyhi,
			    _xc, _yc, _r1)) {
	    double xc(_xc), yc(_yc);
	    double r0sq(_r0*_r0), r1sq(_r1*_r1);
	    for(int j=iylo; j<=iyhi; j++) {
	      double dy  = d.biny(j)-yc;
	      double dy2 = dy*dy;
	      for(int i=ixlo; i<=ixhi; i++) {
		double dx  = d.binx(i)-xc;
		double dx2 = dx*dx;
		double rsq = dx2 + dy2;
		double f   = atan2(dy,dx);
		if ( (rsq >= r0sq && rsq <= r1sq) &&
		     ( (f>=_f0 && f<=_f1) ||
		       (f+2*M_PI <= _f1) ) )
		sum += double(e.content(i,j))-p;
	      }
	    }
	  }
	  else {
            return 0;
          }
	}
	double n = double(e.info(EntryImage::Normalization));
	return n > 0 ? sum / n : sum;
      }
    private:
      const Entry*& _entry;
      double _xc, _yc, _r0, _r1, _f0, _f1;
    };

  };
};

using namespace Ami;

static QChar _integrate(0x002C);
static QChar _range    (0x0023);
const QChar& BinMath::integrate() { return _integrate; }
const QChar& BinMath::range    () { return _range    ; }
const double BinMath::floatPrecision() { return 1.e3; }

BinMath::BinMath(const DescEntry& output, 
		 const char* expr) :
  AbsOperator(AbsOperator::BinMath),
  _cache     (0),
  _term      (0),
  _fterm     (0),
  _entry     (0)
{
  strncpy(_expression, expr, EXPRESSION_LEN);
  memcpy (_desc_buffer, &output, output.size());
}

#define CASETERM(type)							\
  case DescEntry::type:							\
  { t = new Ami::BinMathC::Entry##type##Term(_input,lo,hi);		\
    break; }

BinMath::BinMath(const char*& p, const DescEntry& input, FeatureCache& features) :
  AbsOperator(AbsOperator::BinMath),
  _cache (&features)
{
  _extract(p, _expression , EXPRESSION_LEN);
  _extract(p, _desc_buffer, DESC_LEN);

  const DescEntry& o = *reinterpret_cast<const DescEntry*>(_desc_buffer);

  _entry = EntryFactory::entry(o);
 
  { QString expr(_expression);
    printf("BinMath input expr %s\n",qPrintable(expr));
    QString new_expr;
    // parse expression for bin indices
    QRegExp match("\\[[0-9,]+\\]");
    int last=0;
    int pos=0;
    int mlen=0;
    while( (pos=match.indexIn(expr,pos)) != -1) {
      mlen = match.matchedLength();
      QString use = expr.mid(pos+1,mlen-2);
      unsigned lo, hi;  
      if (!_parseIndices(use,lo,hi)) {
        pos += mlen;
        continue;
      }
      Term* t;
      switch(input.type()) {
	CASETERM(Waveform);
	CASETERM(TH1F);
	CASETERM(Prof);
      case DescEntry::Image:
	{ int ypos = pos+mlen;
	  ypos = match.indexIn(expr,ypos);
	  mlen += match.matchedLength();
	  QString yuse = expr.mid(ypos+1,match.matchedLength()-2);
	  unsigned ylo, yhi;  _parseIndices(yuse,ylo,yhi);
	  if (expr[pos+mlen]=='[') {  // third dimension (azimuth)
	    int zpos = pos+mlen;
	    zpos = match.indexIn(expr,zpos);
	    mlen += match.matchedLength();
	    QString zuse = expr.mid(zpos+1,match.matchedLength()-2);
	    unsigned zlo, zhi;  _parseIndices(zuse,zlo,zhi);
	    t = new Ami::BinMathC::EntryImageTermF(_input,
						   double( lo)/floatPrecision(),double( hi)/floatPrecision(),
						   double(ylo)/floatPrecision(),double(yhi)/floatPrecision(),
						   double(zlo)/floatPrecision(),double(zhi)/floatPrecision()); 
	  }
	  else
	    t = new Ami::BinMathC::EntryImageTerm(_input,lo,hi,ylo,yhi); 
	}
	break;
      default:
	printf("BinMath: No implementation for entry type %d\n",input.type());
	t = 0;
	break;
      }
      new_expr.append(expr.mid(last,pos-last));
      new_expr.append(QString("[%1]").arg((ulong)t,0,16));
      pos += mlen;
      last = pos;
    }
    new_expr.append(expr.mid(last));

//     std::list<Variable*> variables; // none
//     Expression parser(variables);
//     _term = parser.evaluate(new_expr);
    FeatureExpression parser;
    _term = parser.evaluate(features,new_expr);
    if (!_term)
      printf("BinMath failed to parse %s (%s)\n",qPrintable(new_expr),_expression);
  }

  if (o.type() == DescEntry::Prof ||
      o.type() == DescEntry::Scan) {
    QString expr(o.xtitle());
    FeatureExpression parser;
    _fterm = parser.evaluate(features,expr);
    if (!_fterm)
      printf("BinMath failed to parse f %s\n",qPrintable(expr));
  }
}

BinMath::BinMath(const char*& p) :
  AbsOperator(AbsOperator::BinMath),
  _cache     (0),
  _term      (0),
  _fterm     (0),
  _entry     (0)
{
  _extract(p, _expression , EXPRESSION_LEN);
  _extract(p, _desc_buffer, DESC_LEN);
}

BinMath::~BinMath()
{
  if (_term) delete _term;
  if (_fterm) delete _fterm;
  if (_entry) delete _entry;
}

DescEntry& BinMath::output   () const 
{ 
  return _entry ? _entry->desc() : *reinterpret_cast<DescEntry*>(const_cast<char*>(_desc_buffer)); 
}

const char* BinMath::expression() const { return _expression; }

void*      BinMath::_serialize(void* p) const
{
  _insert(p, _expression , EXPRESSION_LEN);
  _insert(p, _desc_buffer, DESC_LEN);
  return p;
}

Entry&     BinMath::_operate(const Entry& e) const
{
  _entry->valid(e.time());
  if (!e.valid())
    return *_entry;

  if (_term) {
    _input = &e;
    double y = _term->evaluate();

    switch(_entry->desc().type()) {
    case DescEntry::Scalar:  
      { EntryScalar* en = static_cast<EntryScalar*>(_entry);
	en->addcontent(y);
	break; }
    case DescEntry::TH1F:
      { EntryTH1F* en = static_cast<EntryTH1F*  >(_entry);
	en->addcontent(1.,y); 
	en->addinfo(1.,EntryTH1F::Normalization);
	break; }
    case DescEntry::Prof:  
      if (!_fterm)
	return *_entry;
     { bool damaged=false; double x=_fterm->evaluate();
	if (!damaged) {
	  EntryProf* en = static_cast<EntryProf*  >(_entry);
	  en->addy(y,x);
	  en->addinfo(1.,EntryProf::Normalization);
	}
	break; }
    case DescEntry::Scan:    
      if (!_fterm)
	return *_entry;
      { bool damaged=false; double x=_fterm->evaluate();
	if (!damaged) {
	  EntryScan* en = static_cast<EntryScan*  >(_entry);
	  en->addy(y,x);
	  en->addinfo(1.,EntryScan::Normalization);
	}
	break; }
    case DescEntry::Waveform:
    case DescEntry::TH2F:
    case DescEntry::Image:
    default:
      printf("BinMath::_operator no implementation for type %d\n",_entry->desc().type());
      break;
    }
  }
  return *_entry;
}

static bool _parseIndices(const QString& use, unsigned& lo, unsigned& hi)
{
  int index = use.indexOf(_integrate);
  if (index == -1) {
    lo = hi = use.toInt();
    if (lo > MAX_INDEX)
      return false;
  }
  else {
    lo = use.mid(0,index).toInt();
    hi = use.mid(index+1,-1).toInt();
    //    if (lo > hi) { unsigned i=lo; lo=hi; hi=i; }
  }
  return true;
}

