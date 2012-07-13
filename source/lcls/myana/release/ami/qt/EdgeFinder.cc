#include "ami/qt/EdgeFinder.hh"

#include "ami/qt/DescTH1F.hh"
#include "ami/qt/DescTH2F.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/CursorDefinition.hh"
#include "ami/qt/EdgeCursor.hh"
#include "ami/qt/EdgePlot.hh"
#include "ami/qt/WaveformDisplay.hh"
#include "ami/qt/PlotFrame.hh"

#include "ami/data/DescTH1F.hh"
#include "ami/data/DescTH2F.hh"
#include "ami/data/DescWaveform.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EdgeFinder.hh"

#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
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

EdgeFinder::EdgeFinder(QWidget* parent,
		       ChannelDefinition* channels[], unsigned nchannels, WaveformDisplay& frame) :
  QtPWidget (parent),
  _channels (channels),
  _nchannels(nchannels),
  _channel  (0),
  _frame    (frame),
  _title    (new QLineEdit("Edge plot")),
  _dead     (new QLineEdit("0"))
{
  _baseline  = new EdgeCursor("baseline" ,*_frame.plot());
  _threshold = new EdgeCursor("threshold",*_frame.plot());

  setWindowTitle("EdgeFinder Plot");
  setAttribute(::Qt::WA_DeleteOnClose, false);

  QComboBox* channelBox = new QComboBox;
  for(unsigned i=0; i<nchannels; i++)
    channelBox->addItem(channels[i]->name());

  _hist   = new DescTH1F ("Sum (1dH)");
  _hist->button()->setChecked(true);

  QHBoxLayout* vsl = new QHBoxLayout;
  _hist2d = new DescTH2F (vsl);
  _hist2d->td_button()->setEnabled(true);
  _hist2d->im_button()->setEnabled(false);
  _hist2d->td_button()->setChecked(true);

  _vtbutton = new QButtonGroup;
  QRadioButton* plotTimeB = new QRadioButton("Time");
  QRadioButton* plotAmplB = new QRadioButton("Amplitude");
  QRadioButton* plotAmplvTimeB = new QRadioButton("Ampl v Time");
  _vtbutton->addButton(plotTimeB,(int)Ami::EdgeFinder::Location);
  _vtbutton->addButton(plotAmplB,(int)Ami::EdgeFinder::Amplitude);
  _vtbutton->addButton(plotAmplvTimeB,(int)Ami::EdgeFinder::AmplvLoc);
  plotTimeB->setChecked(true);

  QPushButton* plotB  = new QPushButton("Plot");
  QPushButton* closeB = new QPushButton("Close");
  
  QVBoxLayout* layout = new QVBoxLayout;
  { QLabel* desc = new QLabel;
    desc->setWordWrap(true);
    desc->setAlignment(::Qt::AlignHCenter);
    desc->setText("Locates leading edge of each pulse above 'threshold' at a constract fraction of peak height from 'baseline'.");
    layout->addWidget(desc); }
  { QGroupBox* channel_box = new QGroupBox("Source Channel");
    QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addWidget(new QLabel("Channel"));
    layout1->addWidget(channelBox);
    layout1->addStretch();
    channel_box->setLayout(layout1);
    layout->addWidget(channel_box); }
  { QGroupBox* locations_box = new QGroupBox("Define EdgeFinder");
    locations_box->setToolTip("Define baseline and threshold values.");
    QVBoxLayout* layout2 = new QVBoxLayout;
    layout2->addWidget(_baseline);
    layout2->addWidget(_threshold);
    { QHBoxLayout *layout3 = new QHBoxLayout;
      layout3->addWidget(new QLabel("edge to find: "));
      _leading = new QCheckBox("Leading");
      _leading->setChecked(true);
      layout3->addWidget(_leading);
      _trailing = new QCheckBox("Trailing");
      layout3->addWidget(_trailing);
      layout3->addStretch();
      layout2->addLayout(layout3); }
    { QHBoxLayout *layout3 = new QHBoxLayout;
      layout3->addWidget(new QLabel("dead time: "));
      layout3->addWidget(_dead);
      layout3->addStretch();
      layout2->addLayout(layout3); }
    locations_box->setLayout(layout2);
    layout->addWidget(locations_box); }
  { QGroupBox* plot_box = new QGroupBox("Plot");
    QVBoxLayout* layout1 = new QVBoxLayout;
    { QHBoxLayout* layout2 = new QHBoxLayout;
      layout2->addWidget(new QLabel("Title"));
      layout2->addWidget(_title);
      layout1->addLayout(layout2); }
    { QHBoxLayout* layout2 = new QHBoxLayout;
      { QVBoxLayout* layout3 = new QVBoxLayout;
        layout3->addWidget(plotTimeB);
        layout3->addWidget(plotAmplB);
        layout2->addLayout(layout3); }
      layout2->addWidget(_hist );
      layout1->addLayout(layout2); }
    { QHBoxLayout* layout2 = new QHBoxLayout;
      layout2->addWidget(plotAmplvTimeB);
      layout2->addWidget(_hist2d);
      layout1->addLayout(layout2); }
    plot_box->setLayout(layout1); 
    layout->addWidget(plot_box); }
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
  
EdgeFinder::~EdgeFinder()
{
}

void EdgeFinder::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert(p, "EdgeCursor", "_baseline", _baseline ->save(p) );
  XML_insert(p, "EdgeCursor", "_threshold",_threshold->save(p) );

  XML_insert(p, "QLineEdit", "_title", QtPersistent::insert(p,_title->text()) );
  XML_insert(p, "QButtonGroup", "_vtbutton", QtPersistent::insert(p,_vtbutton->checkedId()) );
  XML_insert(p, "DescTH1F", "_hist"  , _hist  ->save(p) );
  XML_insert(p, "DescTH2F", "_hist2d", _hist2d->save(p) );

  for(std::list<EdgePlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    XML_insert(p, "EdgePlot", "_plots", (*it)->save(p) );
  }
}

void EdgeFinder::load(const char*& p)
{
  for(std::list<EdgePlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
  _plots.clear();

  XML_iterate_open(p,tag)

    if (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_baseline")
      _baseline ->load(p);
    else if (tag.name == "_threshold")
      _threshold->load(p);
    else if (tag.name == "_title")
      _title->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_vtbutton")
      _vtbutton->button(QtPersistent::extract_i(p))->setChecked(true);
    else if (tag.name == "_hist")
      _hist->load(p);
    else if (tag.name == "_hist2d")
      _hist2d->load(p);
    else if (tag.name == "_plots") {
      EdgePlot* plot = new EdgePlot(this, p);
      _plots.push_back(plot);
      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    }

  XML_iterate_close(EdgeFinder,tag);

  emit changed();
}

void EdgeFinder::save_plots(const QString& p) const
{
  int i=1;
  for(std::list<EdgePlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    QString s = QString("%1_%2.dat").arg(p).arg(i++);
    FILE* f = fopen(qPrintable(s),"w");
    if (f) {
      (*it)->dump(f);
      fclose(f);
    }
  }
}

void EdgeFinder::configure(char*& p, unsigned input, unsigned& output,
			   ChannelDefinition* channels[], int* signatures, unsigned nchannels)
{
  for(std::list<EdgePlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->configure(p,input,output,channels,signatures,nchannels,_frame.xinfo());
}

void EdgeFinder::setup_payload(Cds& cds)
{
  for(std::list<EdgePlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->setup_payload(cds);
}

void EdgeFinder::update()
{
  for(std::list<EdgePlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->update();
}

void EdgeFinder::initialize(const Ami::DescEntry& e)
{
  switch(e.type()) {
  case DescEntry::Waveform:
    { const DescWaveform& wf = reinterpret_cast<const DescWaveform&>(e);
      _hist->lo(wf.xlow());
      _hist->hi(wf.xup()); }
    break;
  default:
    break;
  }
}

void EdgeFinder::set_channel(int c) 
{ 
  _channel=c; 
}

void EdgeFinder::plot()
{
  if (!_leading->isChecked() && !_trailing->isChecked())
    return;

  Ami::DescEntry* desc = 0;
  switch(_vtbutton->checkedId()) {
  case Ami::EdgeFinder::Location:
    desc = new Ami::DescTH1F(qPrintable(_title->text()),
                             "edge location", "pulses",
                             _hist->bins(),_hist->lo(),_hist->hi(),false);  
    break;
  case Ami::EdgeFinder::Amplitude:
    desc = new Ami::DescTH1F(qPrintable(_title->text()),
                             "amplitude", "pulses",
                             _hist->bins(),_hist->lo(),_hist->hi(),false);  
    break;
  case Ami::EdgeFinder::AmplvLoc:
    desc = new Ami::DescTH2F(qPrintable(_title->text()),
                             "location", "amplitude", 
                             _hist2d->xbins(),_hist2d->xlo(),_hist2d->xhi(),
                             _hist2d->ybins(),_hist2d->ylo(),_hist2d->yhi(),
                             false);  
    break;
  default:
    printf("EdgeFinder unknown plot type %d\n",_vtbutton->checkedId());
    return;
  }

  double deadtime = _dead->text().toDouble();
  EdgePlot* plot =
      new EdgePlot(this, _title->text(),_channel,
                   new Ami::EdgeFinder(0.5, _threshold->value(), _baseline ->value(),
                                       Ami::EdgeFinder::EdgeAlgorithm(Ami::EdgeFinder::halfbase2peak,
                                                                      _leading->isChecked()),
                                       deadtime, *desc,
                                       Ami::EdgeFinder::Parameter(_vtbutton->checkedId())));

  if (_leading->isChecked() && _trailing->isChecked()) {
      // Add trailing plot to EdgePlot!
      plot->addfinder(new Ami::EdgeFinder(0.5, _threshold->value(), _baseline ->value(),
                                          Ami::EdgeFinder::EdgeAlgorithm(Ami::EdgeFinder::halfbase2peak,
                                                                         false),
                                          deadtime, *desc,
                                          Ami::EdgeFinder::Parameter(_vtbutton->checkedId())));
  }

  _plots.push_back(plot);

  delete desc;

  connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));

  emit changed();
}

void EdgeFinder::remove_plot(QObject* obj)
{
  EdgePlot* plot = static_cast<EdgePlot*>(obj);
  _plots.remove(plot);

  disconnect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
}

