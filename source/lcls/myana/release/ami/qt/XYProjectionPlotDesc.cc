#include "ami/qt/XYProjectionPlotDesc.hh"

#include "ami/qt/QtPersistent.hh"
#include "ami/qt/RectangleCursors.hh"

#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/XYProjection.hh"

#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

using namespace Ami::Qt;

enum { PlotSum, PlotMean };

XYProjectionPlotDesc::XYProjectionPlotDesc(QWidget* parent,
					   const RectangleCursors& r) :
  QWidget(parent),
  _rectangle(r)
{
  QRadioButton* xaxisB = new QRadioButton("X");
 QRadioButton* yaxisB = new QRadioButton("Y");
  _axis = new QButtonGroup;
  _axis->addButton(xaxisB,0);
  _axis->addButton(yaxisB,1);
  xaxisB->setChecked(true);

  QRadioButton* sumB  = new QRadioButton("sum");
  QRadioButton* meanB = new QRadioButton("mean");
  _norm = new QButtonGroup;
  _norm->addButton(sumB ,PlotSum);
  _norm->addButton(meanB,PlotMean);
  sumB->setChecked(true);

  QVBoxLayout* layout1 = new QVBoxLayout;
  layout1->addStretch();
  { QHBoxLayout* layout2 = new QHBoxLayout;
    layout2->addWidget(new QLabel("Project"));
    { QVBoxLayout* layout3 = new QVBoxLayout;
      layout3->addWidget(sumB);
      layout3->addWidget(meanB);
      layout2->addLayout(layout3); }
    layout2->addWidget(new QLabel("onto"));
    { QVBoxLayout* layout3 = new QVBoxLayout;
      layout3->addWidget(xaxisB);
      layout3->addWidget(yaxisB);
      layout2->addLayout(layout3); }
    layout2->addWidget(new QLabel("axis"));
    layout2->addStretch(); 
    layout1->addLayout(layout2); }
  layout1->addStretch();
  setLayout(layout1);
}

XYProjectionPlotDesc::~XYProjectionPlotDesc()
{
}

void XYProjectionPlotDesc::save(char*& p) const
{
  XML_insert(p, "QButtonGroup", "_axis", QtPersistent::insert(p, _axis->checkedId()) );
  XML_insert(p, "QButtonGroup", "_norm", QtPersistent::insert(p, _norm->checkedId()) );
}

void XYProjectionPlotDesc::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "_axis")
      _axis->button(QtPersistent::extract_i(p))->setChecked(true);
    else if (tag.name == "_norm")
      _norm->button(QtPersistent::extract_i(p))->setChecked(true);
  XML_iterate_close(XYProjectionPlotDesc,tag);
}

Ami::XYProjection* XYProjectionPlotDesc::desc(const char* title) const
{
  Ami::XYProjection* proj;

  if (_axis->checkedId()==0) { // X
    if (_norm->checkedId()==0) {
      Ami::DescTH1F desc(title,
			 "pixel", "sum",
			 _rectangle.nxbins(), _rectangle.xlo(), _rectangle.xhi());
      proj = new Ami::XYProjection(desc, Ami::XYProjection::X, _rectangle.ylo(), _rectangle.yhi());
    }
    else {
      Ami::DescProf desc(title,
			 "pixel", "mean",
			 _rectangle.nxbins(), _rectangle.xlo(), _rectangle.xhi(), "");
      proj = new Ami::XYProjection(desc, Ami::XYProjection::X, _rectangle.ylo(), _rectangle.yhi());
    }
  }
  else { // Y
    if (_norm->checkedId()==0) {
      Ami::DescTH1F desc(title,
			 "pixel", "sum",
			 _rectangle.nybins(),  _rectangle.ylo(), _rectangle.yhi());
      proj = new Ami::XYProjection(desc, Ami::XYProjection::Y, _rectangle.xlo(), _rectangle.xhi());
    }
    else {
      Ami::DescProf desc(title,
			 "pixel", "mean",
			 _rectangle.nybins(), _rectangle.ylo(), _rectangle.yhi(), "");
      proj = new Ami::XYProjection(desc, Ami::XYProjection::Y, _rectangle.xlo(), _rectangle.xhi());
    }
  }
  
  return proj;
}
