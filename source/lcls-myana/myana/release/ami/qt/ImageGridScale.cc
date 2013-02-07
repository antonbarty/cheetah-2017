#include "ami/qt/ImageGridScale.hh"

#include "ami/qt/ImageDisplay.hh"
#include "ami/qt/ImageFrame.hh"
#include "ami/qt/CrossHair.hh"
//#include "ami/qt/CrossHairDelta.hh"

#include "ami/data/Cds.hh"
#include "ami/data/EntryImage.hh"

#include <QtGui/QGridLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>

using namespace Ami::Qt;

enum { Pixels, Millimeters };

ImageGridScale::ImageGridScale(ImageFrame& frame, bool grab) :
  _frame    (frame),
  _scaled   (false),
  _scalex   (1),
  _scaley   (1)
{
  QRadioButton* pixelsB = new QRadioButton("pixels");
  QRadioButton* phyB    = new QRadioButton("mms");
  _group   = new QButtonGroup;
  pixelsB->setChecked(!_scaled);
  phyB   ->setChecked( _scaled);
  _group->addButton(pixelsB,Pixels);
  _group->addButton(phyB   ,Millimeters);

  QVBoxLayout* layout = new QVBoxLayout;
  { QGroupBox* w = new QGroupBox("XY Units");
    w->setAlignment(::Qt::AlignHCenter);
    QHBoxLayout* lh = new QHBoxLayout;
    lh->addWidget(pixelsB);
    lh->addStretch();
    lh->addWidget(phyB);
    w->setLayout(lh);
    layout->addWidget(w); }

  layout->addLayout(_clayout = new QGridLayout);
  _clayout->setVerticalSpacing(0);

  setLayout(layout);

  CrossHair::layoutHeader(*_clayout); _nrows=1;
  _cross_hairs.push_back( new CrossHair(*this, *_clayout, _nrows++, grab) );
  _cross_hairs.push_back( new CrossHair(*this, *_clayout, _nrows++, grab) );
//   _delta = new CrossHairDelta(*_clayout, _nrows++,
// 			      *_cross_hairs.front(),
// 			      *_cross_hairs.back());

  connect(phyB, SIGNAL(toggled(bool)), this, SLOT(phy_scale(bool)));
}
  
ImageGridScale::~ImageGridScale()
{
}

void ImageGridScale::save(char*& p) const
{
}

void ImageGridScale::load(const char*& p) 
{
}

void ImageGridScale::update()
{
}

void ImageGridScale::setup_payload(Cds& cds)
{
  for(int i=0, n=0; n<cds.totalentries(); i++) {
    const Entry* e = cds.entry(i);
    if (e) {
      if (e->desc().type() == Ami::DescEntry::Image) {
	_scalex = static_cast<const DescImage&>(e->desc()).mmppx();
	_scaley = static_cast<const DescImage&>(e->desc()).mmppy();
        if (_scalex==0) {
          _group->button(Pixels     )->setChecked(true);
          _group->button(Millimeters)->setEnabled(false);
          _scaled=false;
        }
        else {
          _group->button(1)->setEnabled(true);
        }
	phy_scale(_scaled);
	return;
      }
      n++;
    }
  }
  printf("ImageGridScale::setup_payload found no image\n");
}

void ImageGridScale::phy_scale(bool v) 
{ 
  _scaled = v;
  for(std::list<CrossHair*>::iterator it=_cross_hairs.begin();
      it!=_cross_hairs.end(); it++)
    if (v)
      (*it)->set_scale(_scalex,_scaley);
    else
      (*it)->set_scale(1.,1.);

  if (v)
    _frame.set_grid_scale(_scalex,_scaley);
  else
    _frame.set_grid_scale(1.,1.);
}

void ImageGridScale::setVisible(bool v)
{
  /*
  for(std::list<CrossHair*>::iterator it=_cross_hairs.begin();
      it!=_cross_hairs.end(); it++)
    (*it)->setVisible(v);
  */
  QWidget::setVisible(v);
}

ImageFrame& ImageGridScale::frame() { return _frame; }

