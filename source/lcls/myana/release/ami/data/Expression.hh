#ifndef Ami_Expression_hh
#define Ami_Expression_hh

#include "ami/service/Exception.hh"

#include <QtCore/QString>

#include <list>

class QChar;

namespace Ami {

  class Term {
  public:
    virtual ~Term() {}
  public:
    virtual double evaluate() const = 0;
  };

  class Variable : public Term {
  public:
    virtual ~Variable() {}
  public:
    virtual Variable*      clone() const = 0;
    virtual const QString& name () const = 0;
  };

  class Constant : public Variable {
  public:
    Constant(const QString& name, double v) : _name(name), _v(v) {}
    ~Constant() {}
  public:
    Variable*      clone   () const { return new Constant(_name,_v); }
    const QString& name    () const { return _name; }
    double         evaluate() const { return _v; }
  private:
    QString _name;
    double  _v;
  };

  class Expression {
  public:
    Expression(const std::list<Variable*>&);
    ~Expression();
  public:
    Term* evaluate(const QString&);
  public:
    static const QChar& exponentiate();
    static const QChar& multiply    ();
    static const QChar& divide      ();
    static const QChar& add         ();
    static const QChar& subtract    ();
    static QString      constant(double);
  private:
    QString& _process(QString&);
    QString& _process(QString&,const QChar&) throw(Event);
  private:
    typedef std::list<Variable*> VarList;
    const VarList& _variables;
  };
};

#endif
		 
