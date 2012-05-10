#include "ami/qt/RPhiProjectionPlotDesc.hh"

#include "ami/qt/QtPersistent.hh"
#include "ami/qt/AnnulusCursors.hh"

#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/RPhiProjection.hh"

#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

#include <math.h>

using namespace Ami::Qt;

static const QChar RHO(0x03c1);
static const QChar PHI(0x03c6);

enum { PlotSum, PlotMean };

RPhiProjectionPlotDesc::RPhiProjectionPlotDesc(QWidget* parent,
					       const AnnulusCursors& r) :
  QWidget(parent),
  _annulus(r)
{
  QRadioButton* raxisB = new QRadioButton(RHO);
  QRadioButton* faxisB = new QRadioButton(PHI);
  _axis = new QButtonGroup;
  _axis->addButton(raxisB,0);
  _axis->addButton(faxisB,1);
  raxisB->setChecked(true);

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
      layout3->addWidget(raxisB);
      layout3->addWidget(faxisB);
      layout2->addLayout(layout3); }
    layout2->addWidget(new QLabel("axis"));
    layout2->addStretch(); 
    layout1->addLayout(layout2); }
  layout1->addStretch();
  setLayout(layout1);
}

RPhiProjectionPlotDesc::~RPhiProjectionPlotDesc()
{
}

void RPhiProjectionPlotDesc::save(char*& p) const
{
  XML_insert(p, "QComboBox", "_axis",QtPersistent::insert(p, _axis->checkedId()) );
  XML_insert(p, "QComboBox", "_norm",QtPersistent::insert(p, _norm->checkedId()) );
}

void RPhiProjectionPlotDesc::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "_axis")
      _axis->button(QtPersistent::extract_i(p))->setChecked(true);
    else if (tag.name == "_norm")
      _norm->button(QtPersistent::extract_i(p))->setChecked(true);
  XML_iterate_close(RPhiProjectionPlotDesc,tag);
}

Ami::RPhiProjection* RPhiProjectionPlotDesc::desc(const char* title) const
{
  Ami::RPhiProjection* proj;

  double f0 = _annulus.phi0();
  double f1 = _annulus.phi1();
  if (!(f0 < f1)) f1 += 2*M_PI;

  if (_axis->checkedId()==0) { // R
    if (_norm->checkedId()==0) {
      Ami::DescTH1F desc(title,
			 "radius", "sum",
			 _annulus.nrbins(), _annulus.r_inner(), _annulus.r_outer());
      proj = new Ami::RPhiProjection(desc, Ami::RPhiProjection::R,
				     f0, f1, 
				     _annulus.xcenter(), _annulus.ycenter());
    }
    else {
      Ami::DescProf desc(title,
			 "radius", "mean",
			 _annulus.nrbins(), _annulus.r_inner(), _annulus.r_outer(), "");
      proj = new Ami::RPhiProjection(desc, Ami::RPhiProjection::R,
				     f0, f1,
				     _annulus.xcenter(), _annulus.ycenter());
    }
  }
  else { // Phi
    int nfbins = int(0.5*(_annulus.r_outer()+_annulus.r_inner())*(f1-f0));
    if (_norm->checkedId()==0) {
      Ami::DescTH1F desc(title, "azimuth", "sum", nfbins, f0, f1);
      proj = new Ami::RPhiProjection(desc, Ami::RPhiProjection::Phi,
				     _annulus.r_inner(), _annulus.r_outer(),
				     _annulus.xcenter(), _annulus.ycenter());
    }
    else {
      Ami::DescProf desc(title, "azimuth", "mean", nfbins, f0, f1, "");
      proj = new Ami::RPhiProjection(desc, Ami::RPhiProjection::Phi,
				     _annulus.r_inner(), _annulus.r_outer(),
				     _annulus.xcenter(), _annulus.ycenter());
    }
  }

//   if (_transform->isChecked())
//     proj->next(new Ami::FFT);
  
  return proj;
}
