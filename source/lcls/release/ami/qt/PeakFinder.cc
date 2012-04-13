#include "PeakFinder.hh"

#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/PeakPlot.hh"
#include "ami/qt/ImageDisplay.hh"
#include "ami/qt/ImageScale.hh"

#include "ami/data/DescImage.hh"

#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QMessageBox>

#include <sys/socket.h>

using namespace Ami::Qt;

PeakFinder::PeakFinder(QWidget* parent,
		       ChannelDefinition* channels[], unsigned nchannels, ImageDisplay& frame) :
  QtPWidget (parent),
  _channels (channels),
  _nchannels(nchannels),
  _channel  (0),
  _frame    (frame)
{
  _threshold = new ImageScale("threshold");

  setWindowTitle("PeakFinder Plot");
  setAttribute(::Qt::WA_DeleteOnClose, false);

  QComboBox* channelBox = new QComboBox;
  for(unsigned i=0; i<nchannels; i++)
    channelBox->addItem(channels[i]->name());

  QPushButton* plotB  = new QPushButton("Plot");
  QPushButton* closeB = new QPushButton("Close");
  
  QVBoxLayout* layout = new QVBoxLayout;
  { QGroupBox* channel_box = new QGroupBox("Source Channel");
    QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addWidget(new QLabel("Channel"));
    layout1->addWidget(channelBox);
    layout1->addStretch();
    channel_box->setLayout(layout1);
    layout->addWidget(channel_box); }
  { QGroupBox* locations_box = new QGroupBox("Counting Threshold");
    locations_box->setToolTip("Define threshold value.");
    QVBoxLayout* layout2 = new QVBoxLayout;
    layout2->addWidget(_threshold);
    locations_box->setLayout(layout2);
    layout->addWidget(locations_box); }
  { QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addStretch();
    layout1->addWidget(plotB);
    layout1->addWidget(closeB);
    layout1->addStretch();
    layout->addLayout(layout1); }
  setLayout(layout);
    
  connect(channelBox, SIGNAL(activated(int)), this, SLOT(set_channel(int)));
  connect(plotB     , SIGNAL(clicked()),      this, SLOT(plot()));
  connect(closeB    , SIGNAL(clicked()),      this, SLOT(hide()));
}
  
PeakFinder::~PeakFinder()
{
}

void PeakFinder::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert(p, "int", "_channel", QtPersistent::insert(p,_channel) );
  XML_insert(p, "QLineEdit", "_threshold_0", QtPersistent::insert(p,_threshold->value(0)) );
  XML_insert(p, "QLineEdit", "_threshold_1", QtPersistent::insert(p,_threshold->value(1)) );

  for(std::list<PeakPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    XML_insert(p, "PeakPlot", "_plots", (*it)->save(p) );
  }
}

void PeakFinder::load(const char*& p)
{
  for(std::list<PeakPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    disconnect(*it, SIGNAL(description_changed()), this, SIGNAL(changed()));
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
  }
  _plots.clear();

  XML_iterate_open(p,tag)
    if      (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_channel")
      _channel = QtPersistent::extract_i(p);
    else if (tag.name == "_threshold_0")
      _threshold->value(0,QtPersistent::extract_d(p));
    else if (tag.name == "_threshold_1")
      _threshold->value(1,QtPersistent::extract_d(p));
    else if (tag.name == "_plots") {
      PeakPlot* plot = new PeakPlot(this, p);
      _plots.push_back(plot);
      connect(plot, SIGNAL(description_changed()), this, SIGNAL(changed()));
      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    }
  XML_iterate_close(PeakFinder,tag);
}

void PeakFinder::save_plots(const QString& p) const
{
}

void PeakFinder::configure(char*& p, unsigned input, unsigned& output,
			   ChannelDefinition* channels[], int* signatures, unsigned nchannels)
{
  for(std::list<PeakPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->configure(p,input,output,channels,signatures,nchannels);
}

void PeakFinder::setup_payload(Cds& cds)
{
  for(std::list<PeakPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->setup_payload(cds);
}

void PeakFinder::update()
{
  for(std::list<PeakPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->update();
}

void PeakFinder::prototype(const DescEntry& e)
{
  _threshold->prototype(e);
}

void PeakFinder::set_channel(int c) 
{ 
  _channel=c; 
}

void PeakFinder::plot()
{
  PeakPlot* plot = new PeakPlot(this,
				QString("%1 Peaks : %2,%3").arg(_channels[_channel]->name())
                                .arg(_threshold->value(0))
                                .arg(_threshold->value(1)),
				_channel,
				_threshold->value(0),
				_threshold->value(1));
  _plots.push_back(plot);

  connect(plot, SIGNAL(description_changed()), this, SIGNAL(changed()));
  connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));

  emit changed();
}

void PeakFinder::remove_plot(QObject* obj)
{
  PeakPlot* plot = static_cast<PeakPlot*>(obj);
  _plots.remove(plot);

  disconnect(plot, SIGNAL(description_changed()), this, SIGNAL(changed()));
  disconnect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
}

