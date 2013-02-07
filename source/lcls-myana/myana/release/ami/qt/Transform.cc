#include "Transform.hh"

#include "TransformConstant.hh"

#include "ami/qt/Calculator.hh"
#include "ami/data/Expression.hh"

#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <fstream>
using std::ifstream;
using std::ofstream;

namespace Ami {
  namespace Qt {
    class TransformAxis : public Variable {
    public:
      TransformAxis(const QString& name, double& v) : _name(name), _v(v) {}
      ~TransformAxis() {}
    public:
      Variable*      clone   () const { return new TransformAxis(_name,_v); }
      const QString& name    () const { return _name; }
      double         evaluate() const { return _v; }
    private:
      QString _name;
      double& _v;
    };
  };
};

using namespace Ami::Qt;

static QChar _exponentiate(0x005E);
static QChar _multiply    (0x00D7);
static QChar _divide      (0x00F7);
static QChar _add         (0x002B);
static QChar _subtract    (0x002D);

Transform::Transform(QWidget*       parent,
		     const QString& title,
		     const QString& axis) :
  QtPWidget (parent),
  _name     (axis),
  _expr     (new QLineEdit),
  _new_name (new QLineEdit),
  _new_value(new QLineEdit),
  _clayout  (new QVBoxLayout)
{
  setWindowTitle(title);
  setAttribute(::Qt::WA_DeleteOnClose, false);

  //  _expr->setFrameShape(QFrame::Box);
  //  _expr->setWordWrap(true);
  _expr->setText(_name);
  _expr->setReadOnly(true);
  _expr->setEnabled(false);
  _term = 0;

  new QDoubleValidator(_new_value);

  QPushButton* addB   = new QPushButton("Add");
  QPushButton* calcB  = new QPushButton("Enter");
  QPushButton* applyB = new QPushButton("Apply");
  QPushButton* clearB = new QPushButton("Clear");
  QPushButton* okB    = new QPushButton("OK");
  QPushButton* cancelB= new QPushButton("Cancel");

  QHBoxLayout* l = new QHBoxLayout;
  QVBoxLayout* layout = new QVBoxLayout;
  { QGroupBox* constants_box = new QGroupBox("Define Constants");
    constants_box->setToolTip("Assign a _named_ variable with a constant _value_.");
    QVBoxLayout* layout2 = _clayout;
    { QGridLayout* layout1 = new QGridLayout;
      layout1->addWidget(new QLabel("Name"),0,0);
      layout1->addWidget(new QLabel("Value"),1,0);
      layout1->addWidget(_new_name ,0,1);
      layout1->addWidget(_new_value,1,1);
      layout1->setColumnStretch(2,1);
      layout1->addWidget(addB,0,3,2,1,::Qt::AlignVCenter);
      layout2->addLayout(layout1); }
    constants_box->setLayout(layout2);
    layout->addWidget(constants_box); }
  { QGroupBox* expr_box = new QGroupBox("Expression:");
    expr_box->setToolTip(QString("Transform the axis coordinate x into x' using named constants and the following operators\n" \
				 "  A %1 B  : A to the power of B\n"\
				 "  A %2 B  : A multiplied by B\n"\
				 "  A %3 B  : A divided by B\n"\
				 "  A %4 B  : A added to B\n"\
				 "  A %5 B  : A subtracted by B\n"\
				 "  where A and B can be a named constant or the axis coordinate name.")
			 .arg(_exponentiate)
			 .arg(_multiply)
			 .arg(_divide)
			 .arg(_add)
			 .arg(_subtract));
    QVBoxLayout* layout2 = new QVBoxLayout;
    { QHBoxLayout* layout1 = new QHBoxLayout;
      layout1->addWidget(new QLabel(_name+QString("\' = ")));
      layout1->addWidget(_expr);
      layout1->addWidget(calcB);
      layout2->addLayout(layout1); }
    { QHBoxLayout* layout1 = new QHBoxLayout;
      layout1->addWidget(applyB);
      layout1->addWidget(clearB);
      layout1->addWidget(okB);
      layout1->addWidget(cancelB);
      layout2->addLayout(layout1); }
    expr_box->setLayout(layout2);
    layout->addWidget(expr_box); }
  l->addLayout(layout);
  l->addStretch();
  setLayout(l);

  connect(addB  , SIGNAL(clicked()), this, SLOT(add()));
  connect(calcB , SIGNAL(clicked()), this, SLOT(calc()));
  connect(applyB, SIGNAL(clicked()), this, SLOT(apply()));
  connect(clearB, SIGNAL(clicked()), this, SLOT(clear()));
  connect(okB   , SIGNAL(clicked()), this, SLOT(apply()));
  connect(okB   , SIGNAL(clicked()), this, SLOT(hide()));
  connect(cancelB, SIGNAL(clicked()), this, SLOT(hide()));
}

Transform::~Transform()
{
  if (_term) delete _term;
}

void Transform::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert(p, "QLineEdit", "_expr", QtPersistent::insert(p,_expr->text()) );

  for(std::list<TransformConstant*>::const_iterator it=_constants.begin(); it!=_constants.end(); it++) {
    XML_insert(p, "TransformConstant", "_constants", (*it)->save(p) );
  }
}

void Transform::load(const char*& p)
{
  for(std::list<TransformConstant*>::const_iterator it=_constants.begin(); it!=_constants.end(); it++)
    delete *it;
  _constants.clear();

  XML_iterate_open(p,tag)
    if      (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_expr")
      _expr->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_constants") {
      TransformConstant* c = new TransformConstant(p);
      _constants.push_back(c);
      _clayout->addWidget(c);
      connect(c, SIGNAL(removed(const QString&)), this, SLOT(remove(const QString&)));
    }
  XML_iterate_close(Transform,tag);

  apply();
}

void Transform::add  ()
{
  TransformConstant* c  = new TransformConstant(_new_name->text(),_new_value->text().toDouble());
  _constants.push_back(c);
  _clayout->addWidget(c);

  connect(c, SIGNAL(removed(const QString&)), this, SLOT(remove(const QString&)));
}

void Transform::remove(const QString& name)
{
  for(std::list<TransformConstant*>::iterator it=_constants.begin(); it!=_constants.end(); it++)
    if ((*it)->name() == name) {
      delete (*it);
      _constants.remove(*it);
      break;
    }
}

void Transform::calc()
{
  QStringList variables;
  variables << _name;
  for(std::list<TransformConstant*>::const_iterator it=_constants.begin(); it!=_constants.end(); it++)
    variables << (*it)->name();

  QStringList ops;
  ops << _exponentiate
      << _multiply
      << _divide  
      << _add     
      << _subtract;

  QStringList none;

  Calculator* c = new Calculator(tr("Y Transform"),_name,
 				 variables, none, ops);
  if (c->exec()==QDialog::Accepted)
    _expr->setText(c->result());

   delete c;
}

void Transform::apply()
{
  std::list<Variable*> variables;
  variables.push_back(new TransformAxis(_name,_axis));
  for(std::list<TransformConstant*>::const_iterator it=_constants.begin(); it!=_constants.end(); it++)
    variables.push_back(new Constant((*it)->name(), (*it)->value()));

  if (_term) delete _term;

  Expression parser(variables);
  QString expr = _expr->text();
  expr.replace(_exponentiate,Expression::exponentiate());
  expr.replace(_multiply    ,Expression::multiply());
  expr.replace(_divide      ,Expression::divide());
  expr.replace(_add         ,Expression::add());
  expr.replace(_subtract    ,Expression::subtract());
  _term = parser.evaluate(expr);
  if (_term==0) {
    QMessageBox::critical(this, QString("Evaluate transform"), QString("Unable to parse expression: %1").arg(expr));
  }

  for(std::list<Variable*>::iterator it=variables.begin(); it!=variables.end(); it++)
    delete (*it);

  emit changed();
}

void Transform::clear()
{
  _expr->setText(_name);
}

double Transform::operator()(double u) const
{
  if (_term) {
    _axis=u;
    return _term->evaluate();
  }
  else return u;
}
