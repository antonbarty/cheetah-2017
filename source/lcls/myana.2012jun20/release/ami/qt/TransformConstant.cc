#include "TransformConstant.hh"

#include "ami/qt/QtPersistent.hh"

#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>

using namespace Ami::Qt;


TransformConstant::TransformConstant(const QString& name, double value) :
  QWidget(0),
  _name  (name), 
  _value (value) 
{
  QPushButton* remB = new QPushButton("Remove");
  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(new QLabel (QString("%1 = %2").arg(name).arg(value)));
  layout->addStretch();
  layout->addWidget(remB);
  setLayout(layout);
  
  connect(remB, SIGNAL(clicked()), this, SLOT(remove()));
}

TransformConstant::TransformConstant(const char*& p) :
  QWidget(0)
{
  load(p);

  QPushButton* remB = new QPushButton("Remove");
  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(new QLabel (QString("%1 = %2").arg(_name).arg(_value)));
  layout->addStretch();
  layout->addWidget(remB);
  setLayout(layout);
  
  connect(remB, SIGNAL(clicked()), this, SLOT(remove()));
}

TransformConstant::~TransformConstant() {}

const QString& TransformConstant::name () const { return _name; }

double         TransformConstant::value() const { return _value; }

void TransformConstant::remove() { emit removed(_name); }

void TransformConstant::save(char*& p) const
{
  XML_insert(p, "QString", "name", QtPersistent::insert(p,name()) );
  XML_insert(p, "QString", "value", QtPersistent::insert(p,value()) );
}

void TransformConstant::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if      (tag.name == "name")
      _name = QtPersistent::extract_s(p);
    else if (tag.name == "value")
      _value = QtPersistent::extract_d(p);
  XML_iterate_close(TransformConstant,tag);
}
