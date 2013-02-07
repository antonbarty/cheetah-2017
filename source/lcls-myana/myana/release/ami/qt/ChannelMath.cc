#include "ami/qt/ChannelMath.hh"

#include "ami/qt/Calculator.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/Filter.hh"
#include "ami/qt/FeatureRegistry.hh"

#include "ami/data/LogicAnd.hh"
#include "ami/data/AbsFilter.hh"
#include "ami/data/AbsOperator.hh"
#include "ami/data/EntryMath.hh"
#include "ami/data/Expression.hh"

#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

using namespace Ami::Qt;

ChannelMath::ChannelMath(const QStringList& names) :
  QWidget (0),
  _expr   (new QLineEdit),
  _changed(false),
  _names  (names),
  _filter (0),
  _operator(0)
{
  _expr->setReadOnly(true);
  QPushButton* calcB = new QPushButton("Enter");

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(new QLabel("Expr"));
  layout->addWidget(_expr);
  layout->addWidget(calcB);
  setLayout(layout);

  connect(calcB, SIGNAL(clicked()), this, SLOT(calc()));
}

ChannelMath::~ChannelMath() 
{
  if (_filter  ) delete _filter;
  if (_operator) delete _operator;
}

static QChar _exponentiate(0x005E);
static QChar _multiply    (0x00D7);
static QChar _divide      (0x00F7);
static QChar _add         (0x002B);
static QChar _subtract    (0x002D);

void ChannelMath::calc()
{
  QStringList ops;
  ops << _exponentiate
      << _multiply
      << _divide
      << _add   
      << _subtract;

  QStringList vops;

  QStringList names(_names);
  names << FeatureRegistry::instance().names();

  Calculator* c = new Calculator(tr("Channel Math"),"",
				 names, vops, ops);
  if (c->exec()==QDialog::Accepted) {
    _expr->setText(c->result());
    _changed = true;
  }

  delete c;
}

bool ChannelMath::resolve(ChannelDefinition* channels[],
			  int*  signatures,
			  int nchannels,
			  const Ami::AbsFilter& f)
{
  QString expr(_expr->text());
  expr.replace(_exponentiate,Expression::exponentiate());
  expr.replace(_multiply    ,Expression::multiply());
  expr.replace(_divide      ,Expression::divide());
  expr.replace(_add         ,Expression::add());
  expr.replace(_subtract    ,Expression::subtract());

  //
  //  Replace all channel names with their signatures
  //
  unsigned chmask=0;
  int pos=0;
  while( pos < expr.length() ) {
    int mlen=0;
    int ich=-1;
    for(int i=0; i<nchannels; i++) {
      const QString& name = channels[i]->name();
      if (expr.mid(pos,name.length())==name && name.length()>mlen) {
	mlen = name.length();
	ich  = i;
      }
    }
    if (mlen) {
      //  Check for dependence upon an undefined channel
      int isig = signatures[ich];
      if (isig<0) 
        return false;
      chmask |= (1<<ich);
      QString newstr = QString("[%1]").arg(signatures[ich]);
      expr.replace(pos,mlen,newstr);
      pos += newstr.length();
    }
    else
      pos++;
  }

  //
  //    Include filters associated with channels used
  //
  if (_filter) delete _filter;
  _filter = f.clone();
  for(int i=0; i<nchannels; i++) {
    if (chmask&(1<<i))
      _filter = new LogicAnd(*_filter, *channels[i]->filter().filter()->clone());
  }

  if (_operator) delete _operator;
  _operator = new EntryMath(qPrintable(expr));

  return true;
}

const Ami::AbsFilter& ChannelMath::filter() const
{
  return *_filter;
}

const Ami::AbsOperator& ChannelMath::op () const
{
  return *_operator;
}

Ami::AbsOperator& ChannelMath::op ()
{
  return *_operator;
}

QString   ChannelMath::expr() const
{
  return _expr->text();
}

void      ChannelMath::expr(const QString& t)
{
  _expr->setText(t);
}
