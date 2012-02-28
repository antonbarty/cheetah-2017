#include "ProjectionPlot.hh"

#include "ami/qt/AxisArray.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/CursorsX.hh"
#include "ami/qt/Filter.hh"
#include "ami/qt/PeakFit.hh"
#include "ami/qt/PlotFactory.hh"
#include "ami/qt/QtTH1F.hh"
#include "ami/qt/Path.hh"
#include "ami/qt/WaveformDisplay.hh"

#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/XYProjection.hh"
#include "ami/data/RPhiProjection.hh"
#include "ami/data/ContourProjection.hh"
#include "ami/data/XYHistogram.hh"

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

ProjectionPlot::ProjectionPlot(QWidget*          parent,
			       const QString&    name,
			       unsigned          input_channel,
			       Ami::AbsOperator* proj) :
  QtPWidget(parent),
  _name    (name),
  _input   (input_channel),
  _proj    (proj),
  _frame   (new WaveformDisplay),
  _showMask(1)
{
  for(int i=0; i<NCHANNELS; i++)
    _channels[i] = new ChannelDefinition(static_cast<QWidget*>(parent), names[i], names, *_frame, color[i], i==0);

  _cursors = new CursorsX(this,_channels,NCHANNELS,*_frame);
  _peakfit = new PeakFit (this,_channels,NCHANNELS,*_frame);

  _layout();
}

ProjectionPlot::ProjectionPlot(QWidget*          parent,
			       const char*&      p) :
  QtPWidget(parent),
  _frame   (new WaveformDisplay)
{
  for(int i=0; i<NCHANNELS; i++)
    _channels[i] = new ChannelDefinition(static_cast<QWidget*>(parent), names[i], names, *_frame, color[i], i==0);
	
  _cursors = new CursorsX(this,_channels,NCHANNELS,*_frame);
  _peakfit = new PeakFit (this,_channels,NCHANNELS,*_frame);

  load(p);

  _layout();
}

ProjectionPlot::~ProjectionPlot()
{
  delete _proj;
}

void ProjectionPlot::_layout()
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
    { QPushButton* cursorsB = new QPushButton("Cursors");
      layout3->addWidget(cursorsB);
      connect(cursorsB, SIGNAL(clicked()), _cursors, SLOT(show())); }
    { QPushButton* peakFitB = new QPushButton("Peak");
      layout3->addWidget(peakFitB);
      connect(peakFitB, SIGNAL(clicked()), _peakfit, SLOT(show())); }
    layout3->addStretch();
    layout->addLayout(layout3); }
  layout->addWidget(_frame);
  setLayout(layout);

  connect(_cursors, SIGNAL(changed()), this, SLOT(update_configuration()));
  connect(_peakfit, SIGNAL(changed()), this, SLOT(update_configuration()));
  show();
}

void ProjectionPlot::save(char*& p) const
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

  XML_insert( p, "WaveformDisplay", "_frame",
              _frame  ->save(p) );
  XML_insert( p, "CursorsX", "_cursors",
              _cursors->save(p) );
  XML_insert( p, "PeakFit", "_peakfit",
              _peakfit->save(p) );

  delete[] buff;
}

void ProjectionPlot::load(const char*& p)
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
      case AbsOperator::XYProjection     : _proj = new XYProjection     (v); break;
      case AbsOperator::RPhiProjection   : _proj = new RPhiProjection   (v); break;
      case AbsOperator::ContourProjection: _proj = new ContourProjection(v); break;
      case AbsOperator::XYHistogram      : _proj = new XYHistogram      (v); break;
      default: _proj=0; printf("Unable to parse projection type %d\n",type); break;
      }
    }
    else if (tag.name == "_frame")
      _frame  ->load(p);
    else if (tag.name == "_cursors")
      _cursors->load(p);
    else if (tag.name == "_peakfit")
      _peakfit->load(p);
  XML_iterate_close(ProjectionPlot,tag);
}

void ProjectionPlot::save_plots(const QString& p) const
{
  _frame  ->save_plots(p);
  _cursors->save_plots(p+"_cursor");
  _peakfit->save_plots(p+"_peakfit");
}

void ProjectionPlot::update_configuration()
{
  emit description_changed();
}

#include "ami/data/Entry.hh"
#include "ami/data/DescEntry.hh"

void ProjectionPlot::setup_payload(Cds& cds)
{
  _frame->reset();

  for(unsigned i=0; i<NCHANNELS; i++)
    _channels[i]->setup_payload(cds);

  _cursors->setup_payload(cds);
  _peakfit->setup_payload(cds);
}

void ProjectionPlot::configure(char*& p, unsigned input, unsigned& output,
			       ChannelDefinition* input_channels[], int* input_signatures, unsigned input_nchannels)
{
  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  ConfigureRequest::Analysis,
						  input_signatures[_input],
						  input = ++output,
						  *input_channels[_input]->filter().filter(),
						  *_proj);
  p += r.size();

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


  _cursors->configure(p,input,output,
		      _channels,signatures,NCHANNELS,
		      ConfigureRequest::Analysis);
  _peakfit->configure(p,input,output,
		      _channels,signatures,NCHANNELS,
		      ConfigureRequest::Analysis);
}

void ProjectionPlot::update()
{
  _frame  ->update();
  _cursors->update();
  _peakfit->update();
}
