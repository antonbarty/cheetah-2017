#ifndef AmiQt_Transform_hh
#define AmiQt_Transform_hh

#include "ami/qt/QtPWidget.hh"
#include "ami/data/AbsTransform.hh"

#include <list>

class QLineEdit;
class QVBoxLayout;
class QLabel;
class QPushButton;
class QString;

namespace Ami {

  class Term;
  
  namespace Qt {

    class TransformConstant;
    class ExprValidator;
    class QtBase;

    class Transform : public QtPWidget,
		      public Ami::AbsTransform {
      Q_OBJECT
    public:
      Transform(QWidget* parent,
		const QString& title,
		const QString& axis);
      ~Transform();
    public:
      double operator()(double) const;
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public slots:
      void add   ();
      void remove(const QString&);
      void calc  ();
      void apply ();
      void clear ();
    signals:
      void changed();
    private:
      QString    _name;
      QLineEdit* _expr;
      QLineEdit* _new_name;
      QLineEdit* _new_value;
      QVBoxLayout* _clayout;
      std::list<TransformConstant*> _constants;

      Ami::Term* _term;
      mutable double _axis;
    };
  };
};

#endif
