#include "DescProf.hh"
#include "ami/qt/QtPersistent.hh"
#include "ami/qt/FeatureRegistry.hh"
#include "ami/qt/FeatureCalculator.hh"

#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QComboBox>
#include <QtGui/QIntValidator>
#include <QtGui/QDoubleValidator>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

using namespace Ami::Qt;


DescProf::DescProf(const char* name, FeatureRegistry* registry) :
  QWidget(0), _button(new QRadioButton(name)), 
  _bins(new QLineEdit("100")), _lo(new QLineEdit("0")), _hi(new QLineEdit("1")),
  _expr(new QLineEdit),
  _registry(registry)
{
  QPushButton* calcB = new QPushButton("X Var");

  _bins->setMaximumWidth(60);
  _lo  ->setMaximumWidth(60);
  _hi  ->setMaximumWidth(60);
  new QIntValidator   (_bins);
  new QDoubleValidator(_lo);
  new QDoubleValidator(_hi);
  QVBoxLayout* layout1 = new QVBoxLayout;
  { QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(_button);
    layout->addWidget(_expr);
    layout->addWidget(calcB);
    layout->addStretch();
    layout1->addLayout(layout); }
  { QHBoxLayout* layout = new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(new QLabel("bins"));
    layout->addWidget(_bins);
    layout->addWidget(new QLabel("lo"));
    layout->addWidget(_lo);
    layout->addWidget(new QLabel("hi"));
    layout->addWidget(_hi);
    layout1->addLayout(layout); }
  setLayout(layout1);

  connect(calcB, SIGNAL(clicked()), this, SLOT(calc()));
}


void DescProf::calc()
{
  FeatureCalculator* c = new FeatureCalculator(tr("X Var Math"),*_registry);
  if (c->exec()==QDialog::Accepted)
    _expr->setText(c->result());

  delete c;
}

QRadioButton* DescProf::button() { return _button; }

QString  DescProf::expr() const { return _expr->text(); }

QString  DescProf::feature() const 
{
  QString e(_expr->text());
  QString new_expr;
  {
    int pos=0;
    const QStringList& slist = _registry->names();
    while(1) {
      int npos = e.size();
      const QString* spos = 0;
      for(QStringList::const_iterator it=slist.begin(); it!=slist.end(); it++) {
	int np = e.indexOf(*it, pos);
	if ((np >=0 && np < npos) || (np==npos && it->size()>spos->size())) {
	  npos = np;
	  spos = &*it;
	}
      }
      if (spos) {
	new_expr.append(e.mid(pos,npos-pos));
	new_expr.append(QString("[%1]").arg(FeatureRegistry::instance().index(*spos)));
	pos = npos + spos->size();
      }
      else 
	break;
      printf("new_expr %s\n",qPrintable(new_expr));
    }
    new_expr.append(e.mid(pos));
  }
  printf("new_expr %s\n",qPrintable(new_expr));
  return new_expr;
}

unsigned DescProf::bins() const { return _bins->text().toInt(); }
double   DescProf::lo  () const { return _lo->text().toDouble(); }
double   DescProf::hi  () const { return _hi->text().toDouble(); }

void DescProf::save(char*& p) const
{
  XML_insert( p, "QString", "_expr",
              QtPersistent::insert(p,_expr ->text()) );
  XML_insert( p, "QString", "_bins",
              QtPersistent::insert(p,_bins->text()) );
  XML_insert( p, "QString", "_lo",
              QtPersistent::insert(p,_lo  ->text()) );
  XML_insert( p, "QString", "_hi",
              QtPersistent::insert(p,_hi  ->text()) );
}

void DescProf::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.element == "QString") {
      if      (tag.name == "_expr")
        _expr->setText(QtPersistent::extract_s(p));
      else if (tag.name == "_bins")
        _bins->setText(QtPersistent::extract_s(p));
      else if (tag.name == "_lo")
        _lo  ->setText(QtPersistent::extract_s(p));
      else if (tag.name == "_hi")
        _hi  ->setText(QtPersistent::extract_s(p));
    }
  XML_iterate_close(DescProf,tag);
}

