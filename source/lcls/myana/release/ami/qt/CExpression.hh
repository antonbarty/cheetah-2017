#ifndef AmiQt_CExpression_hh
#define AmiQt_CExpression_hh

#include <list>

class QChar;
class QString;

namespace Ami {
  class AbsFilter;
  namespace Qt {
    class Condition;
    class CExpression {
    public:
      CExpression(const std::list<Condition*>& conditions);
      ~CExpression();
    public:
      AbsFilter* evaluate(const QString& expression);
    public:
      static const QChar& logicAnd();
      static const QChar& logicOr ();
    private:
      QString& _process(QString& text,const QChar& o);
      QString& _process(QString& text);
    private:
      typedef std::list<Condition*> VarList;
      const VarList& _conditions;
    };
  };
};

#endif
