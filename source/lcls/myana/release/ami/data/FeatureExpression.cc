#include "FeatureExpression.hh"
#include "ami/data/FeatureCache.hh"

#include <QtCore/QRegExp>

using namespace Ami;

Feature::Feature(FeatureCache& f, unsigned index) :
  _features(f),
  _index   (index)
{
}

Feature::~Feature() {}

double Feature::evaluate() const
{
  bool dmg(false);
  double v = _features.cache(_index,&dmg);
  if (dmg) damage(true);
  return v;
}

static bool _dmg;
void Feature::damage(bool l) { _dmg = l; }
bool Feature::damage() { return _dmg; }


FeatureExpression::FeatureExpression() : Expression(_variables) {}

FeatureExpression::~FeatureExpression() {}

Term* FeatureExpression::evaluate(FeatureCache& features,
				  const QString& e)
{
  QString expr;
  { //  Translate feature names to feature cache indices
    //  enclosed within "{}" braces
    int pos=0;
    while(1) {
      int npos = e.size();
      QString spos;
      const std::vector<std::string>& names = features.names();
      for(unsigned i=0; i<features.entries(); i++) {
	QString it(names[i].c_str());
        int np = e.indexOf(it, pos);
        if ((np >=0 && np < npos) || (np==npos && it.size()>spos.size())) {
          npos = np;
          spos = it;
        }
      }
      if (!spos.isEmpty()) {
        expr.append(e.mid(pos,npos-pos));
        expr.append(QString("{%1}").arg(features.lookup(qPrintable(spos))));
	pos = npos + spos.size();
      }
      else
	break;
    }
    expr.append(e.mid(pos));
    printf("expr %s\n",qPrintable(expr));
  }

  QString new_expr;
  { // parse expression for FeatureCache indices
    // and translate into Term pointers enclosed within "[]" brackets
    QRegExp match("\\{[0-9]+\\}");
    int last=0;
    int pos=0;
    while( (pos=match.indexIn(expr,pos)) != -1) {
      QString use = expr.mid(pos+1,match.matchedLength()-2);
      unsigned index = use.toInt();
      Term* t = new Feature(features,index);
      new_expr.append(expr.mid(last,pos-last));
      new_expr.append(QString("[%1]").arg((ulong)t,0,16));
      pos += match.matchedLength();
      last = pos;
    }
    new_expr.append(expr.mid(last));
  }

  return Expression::evaluate(new_expr);
}
