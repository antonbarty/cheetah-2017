#include "ami/qt/ScalarPlotDesc.hh"

#include "ami/qt/DescTH1F.hh"
#include "ami/qt/DescChart.hh"
#include "ami/qt/DescProf.hh"
#include "ami/qt/DescScan.hh"
#include "ami/qt/QtPersistent.hh"

#include "ami/data/DescTH1F.hh"
#include "ami/data/DescScalar.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/DescScan.hh"

#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>

using namespace Ami::Qt;

ScalarPlotDesc::ScalarPlotDesc(QWidget* parent, FeatureRegistry* registry) :
  QWidget(parent)
{
  _title = new QLineEdit  ("title");
  _postB = new QPushButton("Post");
  _postB->setEnabled(false);

  _hist   = new DescTH1F  ("Sum (1dH)");
  _vTime  = new DescChart ("v Time");
  _vFeature = new DescProf("Mean v Var" , registry);
  _vScan    = new DescScan("Mean v Scan", registry);

  _plot_grp = new QButtonGroup;
  _plot_grp->addButton(_hist    ->button(),(int)TH1F);
  _plot_grp->addButton(_vTime   ->button(),(int)vT);
  _plot_grp->addButton(_vFeature->button(),(int)vF);
  _plot_grp->addButton(_vScan   ->button(),(int)vS);
  _hist->button()->setChecked(true);

  _xnorm = new QCheckBox("X");
  _ynorm = new QCheckBox("Y");
  _vnorm = new FeatureList(registry);

  _weightB = new QCheckBox;
  _vweight = new FeatureList(registry);

  QVBoxLayout* layout1 = new QVBoxLayout;
  { QGroupBox* box = new QGroupBox("Plot Type");
    QVBoxLayout* vl = new QVBoxLayout;
    { QHBoxLayout* layout2 = new QHBoxLayout;
      layout2->addWidget(new QLabel("Title"));
      layout2->addWidget(_title);
      layout2->addStretch();
      layout2->addWidget(_postB);
      vl->addLayout(layout2); }
    vl->addWidget(_hist );
    vl->addWidget(_vTime);
    vl->addWidget(_vFeature);
    vl->addWidget(_vScan);
    box->setLayout(vl);
    layout1->addWidget(box); }
  { QGroupBox* box = new QGroupBox("Normalization");
    QHBoxLayout* hl = new QHBoxLayout;
    hl->addStretch();
    hl->addWidget(new QLabel("Normalize"));
    { QVBoxLayout* vl = new QVBoxLayout;
      vl->addWidget(_xnorm);
      vl->addWidget(_ynorm);
      hl->addLayout(vl); }
    hl->addWidget(new QLabel("variable to"));
    hl->addWidget(_vnorm);
    box->setLayout(hl);
    layout1->addWidget(box); }
  { QGroupBox* box = new QGroupBox("Weighted Average");
    QHBoxLayout* hl = new QHBoxLayout;
    hl->addStretch();
    hl->addWidget(_weightB);
    hl->addWidget(new QLabel("Weight by"));
    hl->addWidget(_vweight); 
    hl->addStretch();
    box->setLayout(hl); 
    layout1->addWidget(box); }
  setLayout(layout1);
}

ScalarPlotDesc::~ScalarPlotDesc()
{
}

void ScalarPlotDesc::save(char*& p) const
{
  XML_insert(p, "DescTH1F" , "_hist"    , _hist    ->save(p) );
  XML_insert(p, "DescChart", "_vTime"   , _vTime   ->save(p) );
  XML_insert(p, "DescProf" , "_vFeature", _vFeature->save(p) );
  XML_insert(p, "DescScan" , "_vScan"   , _vScan   ->save(p) );

  XML_insert(p, "QButtonGroup", "_plot_grp", QtPersistent::insert(p,_plot_grp->checkedId ()) );
}

void ScalarPlotDesc::load(const char*& p)
{
  XML_iterate_open(p,tag)
   if (tag.name == "_hist")
      _hist    ->load(p);
    else if (tag.name == "_vTime")
      _vTime   ->load(p);
    else if (tag.name == "_vFeature")
      _vFeature->load(p);
    else if (tag.name == "_vScan")
      _vScan->load(p);
    else if (tag.name == "_plot_grp")
      _plot_grp->button(QtPersistent::extract_i(p))->setChecked(true);
  XML_iterate_close(ScalarPlotDesc,tag);
}

Ami::DescEntry* ScalarPlotDesc::desc(const char* title) const
{
  DescEntry* desc = 0;

  QString vn = QString("(%1)/(%2)").arg(title).arg(_vnorm->entry());

  QString qtitle = title ? QString(title) : _title->text();

  switch(_plot_grp->checkedId()) {
  case ScalarPlotDesc::TH1F:
    { QString v = _ynorm->isChecked() ? vn : qtitle;
      desc = new Ami::DescTH1F(qPrintable(v),qPrintable(v),"events",
			     _hist->bins(),_hist->lo(),_hist->hi()); 
      break; }
  case ScalarPlotDesc::vT: 
    { QString v = _ynorm->isChecked() ? vn : qtitle;
      switch(_vTime->stat()) {
      case Ami::DescScalar::StdDev:
        desc = new Ami::DescScalar(qPrintable(v),"stddev", Ami::DescScalar::StdDev,
                                   _weightB->isChecked() ? qPrintable(_vweight->entry()) : "",
                                   _vTime->pts(),
                                   _vTime->dpt());
        break;
      case Ami::DescScalar::Mean:
      default:
        desc = new Ami::DescScalar(qPrintable(v),"mean", Ami::DescScalar::Mean,
                                   _weightB->isChecked() ? qPrintable(_vweight->entry()) : "",
                                   _vTime->pts(),
                                   _vTime->dpt());
        break; }
      break; }
  case ScalarPlotDesc::vF:
    { QString vy = _ynorm->isChecked() ? vn : qtitle;
      QString vx = _xnorm->isChecked() ? QString("(%1)/(%2)").arg(_vFeature->expr()).arg(_vnorm->entry()) : _vFeature->expr();
      desc = new Ami::DescProf(qPrintable(vy),
			       qPrintable(vx),"mean",
			       _vFeature->bins(),_vFeature->lo(),_vFeature->hi(),"mean",
			       _weightB->isChecked() ? qPrintable(_vweight->entry()) : "");
      break; }
  case ScalarPlotDesc::vS:
    { QString vy = _ynorm->isChecked() ? vn : qtitle;
      QString vx = _xnorm->isChecked() ? QString("(%1)/(%2)").arg(_vScan->expr()).arg(_vnorm->entry()) : _vScan->expr();
      desc = new Ami::DescScan(qPrintable(vy),
			       qPrintable(_vScan->expr()),title,
			       _vScan->bins(),
			       _weightB->isChecked() ? qPrintable(_vweight->entry()) : "");
      break; }
  default:
    desc = 0;
    break;
  }
  return desc;
}

const char* ScalarPlotDesc::expr(const QString& e) const 
{
  static char buf[256];
  QString vn = _ynorm->isChecked() ?
    QString("(%1)/(%2)").arg(e).arg(_vnorm->entry()) : e;
  strncpy(buf, qPrintable(vn), 256);
  return buf;
}

const char* ScalarPlotDesc::title() const
{
  return qPrintable(_title->text());
}

QString ScalarPlotDesc::qtitle() const
{ 
  return _title->text();
}

void ScalarPlotDesc::post(QObject* obj, const char* slot)
{
  connect(_postB, SIGNAL(clicked()), obj, slot);
  _postB->setEnabled(true);
}
