#include "TwoDPlot.hh"

#include "ami/qt/AxisArray.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/CursorsX.hh"
#include "ami/qt/Filter.hh"
#include "ami/qt/PeakFit.hh"
#include "ami/qt/PlotFactory.hh"
#include "ami/qt/Path.hh"
#include "ami/qt/ImageDisplay.hh"
#include "ami/qt/ImageGridScale.hh"
#include "ami/qt/ImageXYProjection.hh"
#include "ami/qt/ImageRPhiProjection.hh"
#include "ami/qt/ImageContourProjection.hh"

#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/DescEntry.hh"
/*
#include "ami/data/XYProjection.hh"
#include "ami/data/RPhiProjection.hh"
#include "ami/data/ContourProjection.hh"
#include "ami/data/XYHistogram.hh"
*/
#include "ami/data/TdcPlot.hh"
#include "ami/data/RawFilter.hh"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QPushButton>

#include "qwt_plot.h"

using namespace Ami::Qt;

static QColor  color[] = { QColor(0,0,255), QColor(255,0,0), QColor(0,255,0), QColor(255,0,255) };
static QStringList names = QStringList() << QString("ChA") << QString("ChB") << QString("ChC") << QString("ChD");

TwoDPlot::TwoDPlot(QWidget*          parent,
                   const QString&    name,
                   Ami::AbsOperator* proj) :
  QtPWidget(parent),
  _name    (name),
  _proj    (proj),
  _frame   (new ImageDisplay),
  _showMask(1)
{
  for(int i=0; i<NCHANNELS; i++)
    _channels[i] = new ChannelDefinition(static_cast<QWidget*>(parent), names[i], names, *_frame, color[i], i==0);
	
  _xyproj  = new ImageXYProjection     (this,_channels,NCHANNELS,*_frame->plot());
  _rfproj  = new ImageRPhiProjection   (this,_channels,NCHANNELS,*_frame->plot());
  _cntproj = new ImageContourProjection(this,_channels,NCHANNELS,*_frame->plot());

  _layout();
}

TwoDPlot::TwoDPlot(QWidget*          parent,
                   const char*&      p) :
  QtPWidget(parent),
  _frame   (new ImageDisplay)
{
  for(int i=0; i<NCHANNELS; i++)
    _channels[i] = new ChannelDefinition(static_cast<QWidget*>(parent), names[i], names, *_frame, color[i], i==0);
	
  _xyproj  = new ImageXYProjection     (this,_channels,NCHANNELS,*_frame->plot());
  _rfproj  = new ImageRPhiProjection   (this,_channels,NCHANNELS,*_frame->plot());
  _cntproj = new ImageContourProjection(this,_channels,NCHANNELS,*_frame->plot());

  load(p);

  _layout();
}

TwoDPlot::~TwoDPlot()
{
  delete _proj;
}

void TwoDPlot::_layout()
{
  setWindowTitle(_name);
  setAttribute(::Qt::WA_DeleteOnClose, true);

  QButtonGroup* showPlotBoxes = new QButtonGroup;
  showPlotBoxes->setExclusive( !_frame->canOverlay() );

  QHBoxLayout* layout = new QHBoxLayout;
  { QVBoxLayout* layout3 = new QVBoxLayout;
    { QGroupBox* chanBox = new QGroupBox("Channels");
      QVBoxLayout* layout1 = new QVBoxLayout;
      QPushButton* chanB[NCHANNELS];
      for(int i=0; i<NCHANNELS; i++) {
	chanB[i] = new QPushButton(names[i]);
	chanB[i]->setPalette(QPalette(color[i]));
	{ QHBoxLayout* layout4 = new QHBoxLayout;
	  QCheckBox* box = new QCheckBox("");
	  showPlotBoxes->addButton(box);
	  connect(box, SIGNAL(toggled(bool)), _channels[i], SLOT(show_plot(bool)));
	  box->setChecked( _showMask & (1<<i) );
	  layout4->addWidget(box);
	  layout4->addWidget(chanB[i]);
	  layout1->addLayout(layout4);
	  connect(chanB[i], SIGNAL(clicked()), _channels[i], SLOT(show()));
	  connect(_channels[i], SIGNAL(changed()), this, SLOT(update_configuration()));
	  connect(_channels[i], SIGNAL(newplot(bool)), box , SLOT(setChecked(bool))); }
      }
      chanBox->setLayout(layout1);
      layout3->addWidget(chanBox); }

    { QPushButton* rectB = new QPushButton("X / Y Selection");
      layout3->addWidget(rectB);
      connect(rectB, SIGNAL(clicked()), _xyproj, SLOT(show())); }

    { QPushButton* cylB = new QPushButton(QString("%1 / %2 Selection").arg(QChar(0x03c1)).arg(QChar(0x03c6)));
      layout3->addWidget(cylB);
      connect(cylB, SIGNAL(clicked()), _rfproj, SLOT(show())); }

    { QPushButton* cntB = new QPushButton("Contour Projection");
      layout3->addWidget(cntB);
      connect(cntB, SIGNAL(clicked()), _cntproj, SLOT(show())); }

    layout3->addStretch();
    layout->addLayout(layout3); }
  layout->addWidget(_frame);
  setLayout(layout);

  connect(_xyproj , SIGNAL(changed()), this, SLOT(update_configuration()));
  connect(_rfproj , SIGNAL(changed()), this, SLOT(update_configuration()));
  connect(_cntproj, SIGNAL(changed()), this, SLOT(update_configuration()));
  show();
}

void TwoDPlot::save(char*& p) const
{
  char* buff = new char[8*1024];

  XML_insert( p, "QtPWidget", "self",
              QtPWidget::save(p) );

  XML_insert( p, "QString", "_name",
              QtPersistent::insert(p,_name) );
  XML_insert( p, "unsigned", "_input", 
              QtPersistent::insert(p,_input) );

  XML_insert( p, "Ami::AbsOperator", "_proj", 
              QtPersistent::insert(p,buff,(char*)_proj->serialize(buff)-buff) );

  for(unsigned i=0; i<NCHANNELS; i++)
    XML_insert( p, "ChannelDefinition", "_channels",
                _channels[i]->save(p) );

  XML_insert( p, "ImageDisplay", "_frame",
              _frame  ->save(p) );
  XML_insert(p, "ImageXYProjection", "_xyproj", _xyproj ->save(p) );
  XML_insert(p, "ImageRPhiProjection", "_rfproj", _rfproj ->save(p) );
  XML_insert(p, "ImageContourProjection", "_cntproj", _cntproj->save(p) );

  delete[] buff;
}

void TwoDPlot::load(const char*& p)
{
  _showMask = 0;
  unsigned nchannels=0;

  XML_iterate_open(p,tag)
    if      (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_name")
      _name  = QtPersistent::extract_s(p);
    else if (tag.name == "_input")
      _input = QtPersistent::extract_i(p);
    else if (tag.name == "_channels") {
      _channels[nchannels]->load(p);
      if (_channels[nchannels]->is_shown())
        _showMask |= 1<<nchannels;
      nchannels++;
    }
    else if (tag.name == "_proj") {
      const char* v = (const char*)QtPersistent::extract_op(p);
      uint32_t type = (AbsOperator::Type)*reinterpret_cast<const uint32_t*>(v);
      v+=2*sizeof(uint32_t); // type and next
      switch(type) {
      /*
      case AbsOperator::XYProjection     : _proj = new XYProjection     (v); break;
      case AbsOperator::RPhiProjection   : _proj = new RPhiProjection   (v); break;
      case AbsOperator::ContourProjection: _proj = new ContourProjection(v); break;
      case AbsOperator::XYHistogram      : _proj = new XYHistogram      (v); break;
      */
      case AbsOperator::TdcPlot          : _proj = new TdcPlot          (v); break;
      default: _proj=0; printf("Unable to parse projection type %d\n",type); break;
      }
    }
    else if (tag.name == "_frame")
      _frame  ->load(p);
    else if (tag.name == "_xyproj")
      _xyproj ->load(p);
    else if (tag.name == "_rfproj")
      _rfproj ->load(p);
    else if (tag.name == "_cntproj")
      _cntproj->load(p);
  XML_iterate_close(TwoDPlot,tag);
}

void TwoDPlot::save_plots(const QString& p) const
{
  _frame  ->save_plots(p);
  _xyproj ->save_plots(p+"_xyproj");
  _rfproj ->save_plots(p+"_rfproj");
  _cntproj->save_plots(p+"_cntproj");
}

void TwoDPlot::update_configuration()
{
  emit description_changed();
}

#include "ami/data/Entry.hh"
#include "ami/data/DescEntry.hh"

void TwoDPlot::setup_payload(Cds& cds)
{
  _frame->reset();

  for(unsigned i=0; i<NCHANNELS; i++)
    _channels[i]->setup_payload(cds);

  _xyproj ->setup_payload(cds);
  _rfproj ->setup_payload(cds);
  _cntproj->setup_payload(cds);
  //  static_cast<ImageDisplay&>(display()).grid_scale().setup_payload(cds);
}

void TwoDPlot::configure(char*& p, unsigned input, unsigned& output)
{
  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  ConfigureRequest::Discovery,
                                                  input,
                                                  ++output,
                                                  RawFilter(),
                                                  *_proj);
  p += r.size();
  input = output;

  int signatures[NCHANNELS];
  for(int i=0; i<NCHANNELS; i++)
    signatures[i] = -1;

  //
  //  Configure channels which depend upon others
  //
  bool lAdded;
  do {
    lAdded=false;
    for(unsigned i=0; i<NCHANNELS; i++) {
      if (signatures[i]<0) {
	int sig = _channels[i]->configure(p,input,output,
					  _channels,signatures,NCHANNELS,
					  ConfigureRequest::Analysis);
	if (sig >= 0) {
	  signatures[i] = sig;
	  lAdded = true;
	  //	    printf("Added signature %d for channel %d\n",sig,i);
	}
      }
    }
  } while(lAdded);


  _xyproj ->configure(p, input, output, _channels,signatures,NCHANNELS);
  _rfproj ->configure(p, input, output, _channels,signatures,NCHANNELS);
  _cntproj->configure(p, input, output, _channels,signatures,NCHANNELS);
}

void TwoDPlot::update()
{
  _frame  ->update();
  _xyproj ->update();
  _rfproj ->update();
  _cntproj->update();
}
