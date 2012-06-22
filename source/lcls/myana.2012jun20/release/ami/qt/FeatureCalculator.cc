#include "ami/qt/FeatureCalculator.hh"

#include "ami/qt/FeatureRegistry.hh"
#include "ami/data/Expression.hh"

using namespace Ami::Qt;

static QChar _exponentiate(0x005E);
static QChar _multiply    (0x00D7);
static QChar _divide      (0x00F7);
static QChar _add         (0x002B);
static QChar _subtract    (0x002D);

static QStringList ops = QStringList() << _exponentiate
					  << _multiply
					     << _divide
						<< _add   
						   << _subtract;

static QStringList vops;

FeatureCalculator::FeatureCalculator(const QString& name, FeatureRegistry& r) :
  Calculator(name, "",
	     r.names(),
	     vops, 
	     ops,
	     r.help())
{
}

QString FeatureCalculator::result()
{
  QString expr(Calculator::result());
  expr.replace(_exponentiate,Expression::exponentiate());
  expr.replace(_multiply    ,Expression::multiply());
  expr.replace(_divide      ,Expression::divide());
  expr.replace(_add         ,Expression::add());
  expr.replace(_subtract    ,Expression::subtract());
  return expr;
}
