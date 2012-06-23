#include "ImageScale.hh"
#include "ami/data/DescEntry.hh"

#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>

using namespace Ami::Qt;

ImageScale::ImageScale(const QString& title) :
  _title(title)
{
  QHBoxLayout* l = new QHBoxLayout;
  l->addWidget(new QLabel(_title));
  l->addStretch(1);
  l->addWidget(_input0=new QLineEdit("0"));
  l->addWidget(new QLabel("ADU"));
  _input0->setMaximumWidth(40);
  new QDoubleValidator(_input0);

  l->addWidget(new QLabel("+"));
  l->addWidget(_input1=new QLineEdit("0"));
  l->addWidget(new QLabel("sigma"));
  _input1->setMaximumWidth(40);
  new QDoubleValidator(_input1);

  setLayout(l);

  connect(this, SIGNAL(changed()), this, SLOT(redo_layout()));
}

ImageScale::~ImageScale() 
{
}

double ImageScale::value(unsigned i) const 
{
  switch(i) {
  case  0: return _input0->text().toDouble();
  case  1: return _input1->text().toDouble();
  default: break;
  }
  return 0;
}

void ImageScale::value(unsigned i,double v)
{
  switch(i) {
  case  0:
    _input0->setText(QString::number(v)); 
    break;
  case  1: 
    _input1->setText(QString::number(v)); 
    break;
  default: break;
  }
}

void ImageScale::prototype(const Ami::DescEntry& e)
{
  _hasGain  = e.hasGainCalib();
  _hasSigma = e.hasRmsCalib();
  _zunits   = QString(e.zunits());
  emit changed();
}

void ImageScale::redo_layout()
{
  QHBoxLayout* l = static_cast<QHBoxLayout*>(layout());
  
  if (_hasGain)
    static_cast<QLabel*>(l->itemAt(3)->widget())->setText(_zunits);

  for(int i=4; i<7; i++) {
    l->itemAt(i)->widget()->setVisible(_hasSigma);
  }
}
