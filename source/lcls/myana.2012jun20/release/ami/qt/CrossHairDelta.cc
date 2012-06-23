#include "ami/qt/CrossHairDelta.hh"

#include "ami/qt/CrossHair.hh"

#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QGridLayout>

#include <math.h>

using namespace Ami::Qt;

CrossHairDelta::CrossHairDelta(QGridLayout& layout, unsigned row,
			       CrossHair& hair1,
			       CrossHair& hair2) :
  _column ( new QLineEdit ),
  _row    ( new QLineEdit ),
  _value  ( new QLineEdit ),
  _hair1  ( hair1 ),
  _hair2  ( hair2 )
{
  _column->setMaximumWidth(40);  _column->setReadOnly(true);
  _row   ->setMaximumWidth(40);  _row   ->setReadOnly(true);
  _value ->setMaximumWidth(40);  _value ->setReadOnly(true);
  
  layout.addWidget(new QLabel(QChar(0x0394)), row, 0, ::Qt::AlignHCenter);
  layout.addWidget(_column, row, 1, ::Qt::AlignHCenter);
  layout.addWidget(_row   , row, 2, ::Qt::AlignHCenter);

  QHBoxLayout* hl = new QHBoxLayout;
  hl->addWidget(new QLabel("D"));
  hl->addWidget(_value);
  layout.addLayout(hl , row, 3, ::Qt::AlignHCenter);

  connect(&hair1, SIGNAL(changed()), this, SLOT(update()));
  connect(&hair2, SIGNAL(changed()), this, SLOT(update()));
}

CrossHairDelta::~CrossHairDelta()
{
}

void CrossHairDelta::update()
{
  double x = _hair1.column()-_hair2.column();
  double y = _hair1.row   ()-_hair2.row   ();
  _column->setText(QString::number(x));
  _row   ->setText(QString::number(y));
  _value ->setText(QString::number(int(0.5+sqrt(x*x+y*y))));
}
