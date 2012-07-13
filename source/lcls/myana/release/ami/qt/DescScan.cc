#include "DescScan.hh"
#include "ami/qt/QtPersistent.hh"
#include "ami/qt/FeatureList.hh"

#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QComboBox>
#include <QtGui/QIntValidator>
#include <QtGui/QLabel>

using namespace Ami::Qt;

DescScan::DescScan(const char* name, FeatureRegistry* registry) :
  QWidget(0), _button(new QRadioButton(name)),
  _bins(new QLineEdit("200"))
{
  _features = new FeatureList(registry);

  _bins->setMaximumWidth(60);
  new QIntValidator   (_bins);

  QVBoxLayout* layout1 = new QVBoxLayout;
  { QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(_button);
    layout->addWidget(_features);
    layout->addStretch();
    layout->addWidget(new QLabel("pts"));
    layout->addWidget(_bins);
    layout1->addLayout(layout); }
  setLayout(layout1);
}

QRadioButton* DescScan::button() { return _button; }
QString  DescScan::expr() const { return _features->entry(); }
QString  DescScan::feature() const { return _features->entry(); }
unsigned DescScan::bins() const { return _bins->text().toInt(); }

void DescScan::save(char*& p) const
{
  XML_insert(p, "FeatureList", "_features", _features->save(p) );
  XML_insert(p, "QLineEdit", "_bins", QtPersistent::insert(p,_bins->text()) );
}

void DescScan::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "_features")
      _features->load(p);
    else if (tag.name == "_bins")
      _bins->setText(QtPersistent::extract_s(p));
  XML_iterate_close(DescScan,tag);
}

