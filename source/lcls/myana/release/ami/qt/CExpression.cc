#include "CExpression.hh"

#include "Condition.hh"

#include "ami/data/FeatureRange.hh"
#include "ami/data/LogicAnd.hh"
#include "ami/data/LogicOr.hh"

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QChar>

#include <math.h>

#define DBUG

using namespace Ami::Qt;

				 

CExpression::CExpression(const std::list<Condition*>& conditions) :
  _conditions(conditions)
{
}

CExpression::~CExpression() {}

static QChar _logicAnd(0x2229);  // "intersection" (upside-down U)
static QChar _logicOr (0x222A);  // "union" (U)

const QChar& CExpression::logicAnd() { return _logicAnd; }
const QChar& CExpression::logicOr () { return _logicOr ; }


QString& CExpression::_process(QString& text,const QChar& o) 
{
  QRegExp termMatch       ("\\[[0-9a-fA-F]+\\]");

  int index, first=-1, last=-1;
  while( (index=text.lastIndexOf(o))!=-1 ) { // left-to-right
    //  while( (index=text.lastIndexOf(o))!=-1 ) { // right-to-left
#ifdef DBUG
    printf("_process(%s) %s\n",qPrintable(QString(o)),qPrintable(text));
#endif
    Ami::AbsFilter* b = 0;
    if (termMatch.indexIn(text,index+1)==index+1) {
      int len = termMatch.matchedLength();
      last = index+len;
      QString str = text.mid(index+2,len-2);
      unsigned ub = str.toULong(0,16);
      b = reinterpret_cast<AbsFilter*>(ub);
#ifdef DBUG
      printf("Found term %s : %p %x\n",qPrintable(str),b,ub);
#endif
    }
    else if (text[index+1].isLetter()) {  // condition
      // loop through variable names and test against this part of the string
      last=-1;
      QString str = text.mid(index+1);
      for(VarList::const_iterator it = _conditions.begin(); it != _conditions.end(); it++) {
	if (str.startsWith((*it)->name())) {
	  b = (*it)->clone();
	  last = index+(*it)->name().size();
#ifdef DBUG
	  printf("Found condition %s at %s\n",qPrintable((*it)->name()),qPrintable(str));
#endif
	  break;
	}
      }
      if (last<0) {
	printf("Failed to find condition at %s\n",qPrintable(str));
	text.clear();
	return text;
      }
    }
    else {
      printf("Unrecognized input at %s\n",qPrintable(text.mid(index+1)));
    }
    Ami::AbsFilter* a=0;
    if ((termMatch.lastIndexIn(text,index-1)+termMatch.matchedLength())==index) {
      int len = termMatch.matchedLength();
      first = index-len;
      QString str = text.mid(first+1,len-2);
      unsigned ua = str.toULong(0,16);
      a = reinterpret_cast<AbsFilter*>(ua);
#ifdef DBUG
      printf("Found term %s : %p %x\n",qPrintable(str),a,ua);
#endif
    }
    else if (text[index-1].isLetter()) {  // condition
      // loop through variable names and test against this part of the string
      first=-1;
      for(VarList::const_iterator it = _conditions.begin(); it != _conditions.end(); it++) {
	QString str = text.mid(index-(*it)->name().size());
	if (str.startsWith((*it)->name())) {
	  a = (*it)->clone();
	  first = index-(*it)->name().size();
#ifdef DBUG
	  printf("Found condition %s at %s\n",qPrintable((*it)->name()),qPrintable(str));
#endif
	  break;
	}
      }
      if (first<0) {
	printf("Failed to find variable at %s\n",qPrintable(text.mid(0,index)));
	text.clear();
	return text;
      }
    }
    else {
      printf("Unrecognized input at %s\n",qPrintable(text.mid(0,index-1)));
    }
    text.remove(first,last-first+1);
    Ami::AbsFilter* B = (o == _logicAnd) ?
      (Ami::AbsFilter*)new Ami::LogicAnd(*a,*b) :
      (Ami::AbsFilter*)new Ami::LogicOr (*a,*b);

    text.insert(first,QString("[%1]").arg((unsigned long)B,0,16));
  }
  return text;
}

QString& CExpression::_process(QString& text)
{
  _process(text,_logicAnd);
  _process(text,_logicOr );
  return text;
}
	 
Ami::AbsFilter* CExpression::evaluate(const QString& expression)
{
  QString text(expression);
#ifdef DBUG
  printf("_evaluate %s\n",qPrintable(text));
#endif

  // 
  //  Special case is the monomial (no operators)
  //
  for(VarList::const_iterator it = _conditions.begin(); it != _conditions.end(); it++) {
    if (text.compare((*it)->name())==0)
      return (Ami::AbsFilter*)(*it)->clone();
  }

  //  process "()" first
  int end;
  while( (end=text.indexOf(')'))!=-1 ) {
    int start=text.lastIndexOf('(',end);
    QString t = text.mid(start+1,end-start-1);
    text.replace(start,end-start+1,_process(t));
  }

  _process(text);

  bool ok;
  Ami::AbsFilter* t = reinterpret_cast<Ami::AbsFilter*>(text.mid(1,text.size()-2).toLong(&ok,16));
#ifdef DBUG
  printf("Result is (%s) %p\n",ok ? "OK" : "Not OK", t);
#endif
  return ok ? t : 0;
}

