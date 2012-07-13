#include "DescTH2F.hh"
#include "ami/qt/QtPersistent.hh"

#include <QtGui/QRadioButton>
#include <QtGui/QLineEdit>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QIntValidator>
#include <QtGui/QDoubleValidator>
#include <QtGui/QLabel>

using namespace Ami::Qt;

DescTH2F::DescTH2F(QLayout* v) :
  QWidget(0), 
  _td_button(new QRadioButton("TH2F")), 
  _im_button(new QRadioButton("Image")), 
  _xbins(new QLineEdit("100")),
  _xlo  (new QLineEdit("0")),
  _xhi  (new QLineEdit("0")),
  _ybins(new QLineEdit("100")),
  _ylo  (new QLineEdit("0")),
  _yhi  (new QLineEdit("0"))
{
  _xbins->setMaximumWidth(60);
  _xlo  ->setMaximumWidth(60);
  _xhi  ->setMaximumWidth(60);
  _ybins->setMaximumWidth(60);
  _ylo  ->setMaximumWidth(60);
  _yhi  ->setMaximumWidth(60);
  new QIntValidator   (1,100000,_xbins);
  new QDoubleValidator(_xlo);
  new QDoubleValidator(_xhi);
  new QIntValidator   (1,100000,_ybins);
  new QDoubleValidator(_ylo);
  new QDoubleValidator(_yhi);
  QHBoxLayout* layout = new QHBoxLayout;
  { QVBoxLayout* vl = new QVBoxLayout;
    vl->addWidget(_td_button);
    vl->addWidget(_im_button);
    layout->addLayout(vl); }
  layout->addStretch();
  { QVBoxLayout* vl = new QVBoxLayout;
    vl->addLayout(v);
    { QHBoxLayout* hl = new QHBoxLayout;
      hl->addWidget(new QLabel("xbins"));
      hl->addWidget(_xbins);
      hl->addWidget(new QLabel("xlo"));
      hl->addWidget(_xlo);
      hl->addWidget(new QLabel("xhi"));
      hl->addWidget(_xhi);
      vl->addLayout(hl); }
    { QHBoxLayout* hl = new QHBoxLayout;
      hl->addWidget(new QLabel("ybins"));
      hl->addWidget(_ybins);
      hl->addWidget(new QLabel("ylo"));
      hl->addWidget(_ylo);
      hl->addWidget(new QLabel("yhi"));
      hl->addWidget(_yhi);
      vl->addLayout(hl); }
    layout->addLayout(vl); }
  setLayout(layout);
  validate();

  connect(_xlo, SIGNAL(editingFinished()), this, SLOT(validate()));
  connect(_xhi, SIGNAL(editingFinished()), this, SLOT(validate()));
  connect(_ylo, SIGNAL(editingFinished()), this, SLOT(validate()));
  connect(_yhi, SIGNAL(editingFinished()), this, SLOT(validate()));
}

QRadioButton* DescTH2F::td_button() { return _td_button; }
QRadioButton* DescTH2F::im_button() { return _im_button; }
unsigned DescTH2F::xbins() const { return _xbins->text().toInt(); }
double   DescTH2F::xlo  () const { return _xlo->text().toDouble(); }
double   DescTH2F::xhi  () const { return _xhi->text().toDouble(); }
unsigned DescTH2F::ybins() const { return _ybins->text().toInt(); }
double   DescTH2F::ylo  () const { return _ylo->text().toDouble(); }
double   DescTH2F::yhi  () const { return _yhi->text().toDouble(); }

void DescTH2F::xbins(unsigned b) { _xbins->setText(QString::number(b)); }
void DescTH2F::xlo  (double   v) { _xlo  ->setText(QString::number(v)); }
void DescTH2F::xhi  (double   v) { _xhi  ->setText(QString::number(v)); }
void DescTH2F::ybins(unsigned b) { _ybins->setText(QString::number(b)); }
void DescTH2F::ylo  (double   v) { _ylo  ->setText(QString::number(v)); }
void DescTH2F::yhi  (double   v) { _yhi  ->setText(QString::number(v)); }

void DescTH2F::validate()
{
  QPalette p(palette());
  if (xlo() < xhi())
    p.setColor(QPalette::Text, QColor(0,0,0));
  else
    p.setColor(QPalette::Text, QColor(0xc0,0,0));

  _xlo->setPalette(p);
  _xhi->setPalette(p);

  if (ylo() < yhi())
    p.setColor(QPalette::Text, QColor(0,0,0));
  else
    p.setColor(QPalette::Text, QColor(0xc0,0,0));

  _ylo->setPalette(p);
  _yhi->setPalette(p);
}

void DescTH2F::save(char*& p) const
{
  XML_insert( p, "QString", "_xbins",
              QtPersistent::insert(p,_xbins->text()) );
  XML_insert( p, "QString", "_xlo",
              QtPersistent::insert(p,_xlo  ->text()) );
  XML_insert( p, "QString", "_xhi",
              QtPersistent::insert(p,_xhi  ->text()) );
  XML_insert( p, "QString", "_ybins",
              QtPersistent::insert(p,_ybins->text()) );
  XML_insert( p, "QString", "_ylo",
              QtPersistent::insert(p,_ylo  ->text()) );
  XML_insert( p, "QString", "_yhi",
              QtPersistent::insert(p,_yhi  ->text()) );
}

void DescTH2F::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if      (tag.name == "_xbins")
      _xbins->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_xlo")
      _xlo  ->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_xhi")
      _xhi  ->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_ybins")
      _ybins->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_ylo")
      _ylo  ->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_yhi")
      _yhi  ->setText(QtPersistent::extract_s(p));
  XML_iterate_close(DescTH2F,tag);
  validate();
}
