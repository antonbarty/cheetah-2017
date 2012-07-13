#include "Condition.hh"

#include "ami/data/FeatureRange.hh"

#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>

using namespace Ami::Qt;


Condition::Condition(const QString& name, 
		     const QString& label,
		     FeatureRange* value) :
  QWidget(0),
  _name  (name), 
  _label (label),
  _value (value) 
{
  QPushButton* remB = new QPushButton("Remove");
  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(new QLabel (label));
  layout->addStretch();
  layout->addWidget(remB);
  setLayout(layout);
  
  connect(remB, SIGNAL(clicked()), this, SLOT(remove()));
}

Condition::~Condition() { delete _value; }

const QString& Condition::name () const { return _name; }

const QString& Condition::label() const { return _label; }

Ami::FeatureRange*  Condition::clone() const
{
  return new Ami::FeatureRange(*_value);
}

void Condition::remove() { emit removed(_name); }

