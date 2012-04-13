#include "Filter.hh"

#include "Condition.hh"
#include "CExpression.hh"

#include "ami/data/RawFilter.hh"
#include "ami/data/FeatureRange.hh"
#include "ami/qt/FeatureRegistry.hh"
#include "ami/qt/Calculator.hh"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QRegExpValidator>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <fstream>
using std::ifstream;
using std::ofstream;

namespace Ami {
  namespace Qt {
  };
};

using namespace Ami::Qt;

Filter::Filter(QWidget* parent,const QString& title) :
  QtPWidget (parent),
  _name     (title),
  _expr     (new QLineEdit),
  _cond_name(new QLineEdit("A")),
  _features (new QComboBox),
  _lo_rng   (new QLineEdit("0")),
  _hi_rng   (new QLineEdit("1")),
  _clayout  (new QVBoxLayout),
  _filter   (new RawFilter)
{
  setWindowTitle(title);
  setAttribute(::Qt::WA_DeleteOnClose, false);

  _cond_name->setMaximumWidth(60);
  _lo_rng   ->setMaximumWidth(60);
  _hi_rng   ->setMaximumWidth(60);
  new QRegExpValidator(QRegExp("[a-zA-Z]+"),_cond_name);
  new QDoubleValidator(_lo_rng);
  new QDoubleValidator(_hi_rng);

  QPushButton* addB   = new QPushButton("Add");
  QPushButton* calcB  = new QPushButton("Enter");
  QPushButton* applyB = new QPushButton("Apply");
  QPushButton* clearB = new QPushButton("Clear");
  QPushButton* okB    = new QPushButton("OK");
  QPushButton* cancelB= new QPushButton("Cancel");

  QHBoxLayout* l = new QHBoxLayout;
  QVBoxLayout* layout = new QVBoxLayout;
  { QGroupBox* conditions_box = new QGroupBox("Define Conditions");
    conditions_box->setToolTip("A CONDITION is an inclusive range of one of the predefined observables." \
			       "The CONDITION is defined by expression " \
			       "[name] := [lo value] <= [observable] <= [hi value]" );
    QVBoxLayout* layout2 = _clayout;
    { QHBoxLayout* layout1 = new QHBoxLayout;
      layout1->addWidget(_cond_name);
      layout1->addWidget(new QLabel(" := "));
      layout1->addWidget(_lo_rng);
      layout1->addWidget(new QLabel("<="));
      //      layout1->addWidget(_bld_box);
      layout1->addWidget(_features);
      layout1->addWidget(new QLabel("<="));
      layout1->addWidget(_hi_rng);
      layout1->addStretch();
      layout1->addWidget(addB);
      layout2->addLayout(layout1); }
    conditions_box->setLayout(layout2);
    layout->addWidget(conditions_box); }
  { QGroupBox* expr_box = new QGroupBox(QString("Expression"));
    expr_box->setToolTip(QString("An EXPRESSION is a set of CONDITIONS separated by the operators\n " \
				 "  A %1 B : logical AND of A and B \n"	\
				 "  A %2 B : logical OR of A and B \n").arg(CExpression::logicAnd()).arg(CExpression::logicOr()));
    QVBoxLayout* layout2 = new QVBoxLayout;
    { QHBoxLayout* layout1 = new QHBoxLayout;
      layout1->addWidget(_expr); _expr->setReadOnly(true);
      layout1->addWidget(calcB);
      layout2->addLayout(layout1); }
    { QHBoxLayout* layout1 = new QHBoxLayout;
      layout1->addStretch();
      layout1->addWidget(applyB);
      layout1->addWidget(clearB);
      layout1->addWidget(okB);
      layout1->addWidget(cancelB);
      layout1->addStretch();
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
  connect(&FeatureRegistry::instance(), SIGNAL(changed()),
	  this  , SLOT(update_features()));
}

Filter::~Filter()
{
  if (_filter) delete _filter;
}

void Filter::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert(p, "QLineEdit", "_expr", QtPersistent::insert(p,_expr->text()) );
  for(std::list<Condition*>::const_iterator it=_conditions.begin(); it!=_conditions.end(); it++) {
    XML_insert(p, "Condition", "_conditions", QtPersistent::insert(p,(*it)->label()) );
  }
}

void Filter::load(const char*& p)
{
  for(std::list<Condition*>::const_iterator it=_conditions.begin(); it!=_conditions.end(); it++)
    delete *it;
  _conditions.clear();
    
  XML_iterate_open(p,tag)
    if (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_expr")
      _expr->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_conditions") {
      QString name = QtPersistent::extract_s(p);
      
      char condition[64],variable[64];
      double lo, hi;
      sscanf(qPrintable(name),"%s := %lg <= %s <= %lg", condition, &lo, variable, &hi);

      printf("new condition %s := %g <= %s <= %g\n",
             condition, lo, variable, hi);
      int index = _features->findText(variable);
      if (index<0)
        printf("Unable to identify %s\n",variable);

      Condition* c = new Condition(condition,
                                   QString("%1 := %2 <= %3 <= %4")
                                   .arg(condition)
                                   .arg(lo)
				   .arg(variable)
                                   .arg(hi),
                                   new FeatureRange(variable,lo,hi));
      _conditions.push_back(c);
      _clayout->addWidget(c);
      connect(c, SIGNAL(removed(const QString&)), this, SLOT(remove(const QString&)));
    }
  XML_iterate_close(Filter,tag);
  _apply();
}

void Filter::add  ()
{
  for(std::list<Condition*>::const_iterator it=_conditions.begin(); it!=_conditions.end(); it++)
    if ((*it)->name()==_cond_name->text()) {
      QMessageBox::critical(this, "Define Condition",
			    QString("Condition name %1 is not unique.").arg(_cond_name->text()));
      return;
    }

  Condition* c  = new Condition(_cond_name->text(),
				QString("%1 := %2 <= %3 <= %4")
				.arg(_cond_name->text())
				.arg(_lo_rng->text())
				.arg(_features->currentText())
				.arg(_hi_rng->text()),
				new FeatureRange(qPrintable(_features->currentText()),
						 _lo_rng->text().toDouble(),
						 _hi_rng->text().toDouble()));
  _conditions.push_back(c);
  _clayout->addWidget(c);

  connect(c, SIGNAL(removed(const QString&)), this, SLOT(remove(const QString&)));
}

void Filter::remove(const QString& name)
{

  for(std::list<Condition*>::iterator it=_conditions.begin(); it!=_conditions.end(); it++)
    if ((*it)->name() == name) {
      delete (*it);
      _conditions.remove(*it);
      break;
    }
}

void Filter::_apply()
{
  if (_filter) delete _filter;

  if (_expr->text().isEmpty()) {
    _filter = new RawFilter;
    return;
  }

  CExpression parser(_conditions);
  _filter = parser.evaluate(_expr->text());

  if (_filter==0) {
    printf("Filter parser failed to evaluate %s\nResetting filter.\n",qPrintable(_expr->text()));
    clear();
  }
}

void Filter::apply()
{
  _apply();
  emit changed();
}

void Filter::clear()
{
  _expr->clear();

  if (_filter) delete _filter;
  _filter = new RawFilter;
}

void Filter::update_features()
{
  _features->clear();
  QStringList names = FeatureRegistry::instance().names();
  if (!names.isEmpty())
    _features->addItems(names);
}

const Ami::AbsFilter* Filter::filter() const { return _filter; }

void Filter::calc()
{
  QStringList conditions;
  for(std::list<Condition*>::const_iterator it=_conditions.begin();
      it != _conditions.end(); it++)
    conditions << (*it)->name();

  QStringList ops;
  ops << CExpression::logicAnd() << CExpression::logicOr();
  
  QStringList none;

  Calculator* c = new Calculator(tr("Filter Expression"),"",
 				 conditions, ops, none);
  if (c->exec()==QDialog::Accepted)
    _expr->setText(c->result());

   delete c;
}
