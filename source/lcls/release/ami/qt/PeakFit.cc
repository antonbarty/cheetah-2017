#include "PeakFit.hh"

#include "ami/qt/DescTH1F.hh"
#include "ami/qt/DescProf.hh"
#include "ami/qt/DescScan.hh"
#include "ami/qt/DescChart.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/EdgeCursor.hh"
#include "ami/qt/PeakFitPlot.hh"
#include "ami/qt/WaveformDisplay.hh"
#include "ami/qt/Calculator.hh"
#include "ami/qt/PlotFrame.hh"

#include "ami/data/DescScalar.hh"
#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/DescScan.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryFactory.hh"
#include "ami/data/PeakFitPlot.hh"

#include "ami/data/Integral.hh"

#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtCore/QRegExp>
#include <QtGui/QRegExpValidator>
#include <QtGui/QMessageBox>

#include <sys/socket.h>

// works for labels not buttons
//#define bold(t) "<b>" #t "</b>"
#define bold(t) #t

enum { _TH1F, _vT, _vF, _vS };


using namespace Ami::Qt;

PeakFit::PeakFit(QWidget* parent, ChannelDefinition* channels[], unsigned nchannels, WaveformDisplay& frame) :
  QtPWidget (parent),
  _channels (channels),
  _nchannels(nchannels),
  _channel  (0),
  _frame    (frame),
  _title    (new QLineEdit("Peak plot"))
{
  setWindowTitle("PeakFit Plot");
  setAttribute(::Qt::WA_DeleteOnClose, false);

  QComboBox* channelBox = new QComboBox;
  for(unsigned i=0; i<nchannels; i++)
    channelBox->addItem(channels[i]->name());

  _baseline  = new EdgeCursor("baseline" ,*_frame.plot());

  QStringList q;
  for(unsigned k=0; k<Ami::PeakFitPlot::NumberOf; k++)
    q << Ami::PeakFitPlot::name((Ami::PeakFitPlot::Parameter)k);
  QComboBox* qtyBox = new QComboBox;
  qtyBox->addItems(q);

  _hist   = new DescTH1F (bold(Sum (1dH)));
  _vTime  = new DescChart(bold(Mean v Time));
  _vFeature = new DescProf (bold(Mean v Var) );
  _vScan    = new DescScan (bold(Mean v Scan));

  _plot_grp = new QButtonGroup;
  _plot_grp->addButton(_hist    ->button(),_TH1F);
  _plot_grp->addButton(_vTime   ->button(),_vT);
  _plot_grp->addButton(_vFeature->button(),_vF);
  _plot_grp->addButton(_vScan   ->button(),_vS);
  _hist->button()->setChecked(true);

  QPushButton* plotB  = new QPushButton("Plot");
  QPushButton* closeB = new QPushButton("Close");
  
  QVBoxLayout* layout = new QVBoxLayout;
  { QLabel* desc = new QLabel;
    desc->setWordWrap(true);
    desc->setAlignment(::Qt::AlignHCenter);
    desc->setText("Finds position, height, and width of largest peak from 'baseline'.");
    layout->addWidget(desc); }
  { QGroupBox* channel_box = new QGroupBox("Source Channel");
    QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addWidget(new QLabel("Channel"));
    layout1->addWidget(channelBox);
    layout1->addStretch();
    channel_box->setLayout(layout1);
    layout->addWidget(channel_box); }
  { QGroupBox* locations_box = new QGroupBox("Define PeakFit");
    locations_box->setToolTip("Define baseline value.");
    QVBoxLayout* layout2 = new QVBoxLayout;
    layout2->addWidget(_baseline);
    locations_box->setLayout(layout2);
    layout->addWidget(locations_box); }
  { QGroupBox* plot_box = new QGroupBox("Plot");
    QVBoxLayout* layout1 = new QVBoxLayout;
    { QHBoxLayout* layout2 = new QHBoxLayout;
      layout2->addWidget(new QLabel("Title"));
      layout2->addWidget(_title);
      layout1->addLayout(layout2); }
    { QHBoxLayout* layout2 = new QHBoxLayout;
      layout2->addWidget(new QLabel("Quantity"));
      layout2->addWidget(qtyBox);
      layout1->addLayout(layout2); }
    layout1->addWidget(_hist );
    layout1->addWidget(_vTime);
    layout1->addWidget(_vFeature);
    layout1->addWidget(_vScan);
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
  connect(qtyBox    , SIGNAL(activated(int)), this, SLOT(set_quantity(int)));
  connect(closeB    , SIGNAL(clicked()),      this, SLOT(hide()));

  set_quantity(0);
}
  
PeakFit::~PeakFit()
{
}

void PeakFit::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert(p, "QLineEdit", "_baseline", QtPersistent::insert(p,_baseline ->value()) );
  XML_insert(p, "QLineEdit", "_title", QtPersistent::insert(p,_title->text()) );
  XML_insert(p, "DescTH1F", "_hist", _hist->save(p) );
  XML_insert(p, "DescChart", "_vTime", _vTime->save(p) );
  XML_insert(p, "DescProf", "_vFeature", _vFeature->save(p) );
  XML_insert(p, "DescScan", "_vScan", _vScan->save(p) );

  for(std::list<PeakFitPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    XML_insert(p, "PeakFitPlot", "_plots", (*it)->save(p) );
  }
}

void PeakFit::load(const char*& p)
{
  for(std::list<PeakFitPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    delete *it;
  }
  _plots.clear();

  XML_iterate_open(p,tag)
    if (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_baseline")
      _baseline->value(QtPersistent::extract_d(p));
    else if (tag.name == "_title")
      _title->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_hist")
      _hist->load(p);
    else if (tag.name == "_vTime")
      _vTime->load(p);
    else if (tag.name == "_vFeature")
      _vFeature->load(p);
    else if (tag.name == "_vScan")
      _vScan->load(p);
    else if (tag.name == "_plots") {
      PeakFitPlot* plot = new PeakFitPlot(this, p);
      _plots.push_back(plot);
      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    }
  XML_iterate_close(PeakFit,tag);
}

void PeakFit::save_plots(const QString& p) const
{
  int i=1;
  for(std::list<PeakFitPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    QString s = QString("%1_%2.dat").arg(p).arg(i++);
    FILE* f = fopen(qPrintable(s),"w");
    if (f) {
      (*it)->dump(f);
      fclose(f);
    }
  }
}

void PeakFit::configure(char*& p, unsigned input, unsigned& output,
			 ChannelDefinition* channels[], int* signatures, unsigned nchannels,
			 ConfigureRequest::Source source)
{
  for(std::list<PeakFitPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->configure(p,input,output,channels,signatures,nchannels,_frame.xinfo(),source);
}

void PeakFit::setup_payload(Cds& cds)
{
  for(std::list<PeakFitPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->setup_payload(cds);
}

void PeakFit::update()
{
  for(std::list<PeakFitPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++)
    (*it)->update();
}

void PeakFit::set_channel(int c) 
{ 
  _channel=c; 
}

void PeakFit::set_quantity(int q)
{
  _quantity=q;
}

void PeakFit::plot()
{
  DescEntry* desc;
  const char* name = Ami::PeakFitPlot::name((Ami::PeakFitPlot::Parameter)_quantity);
  switch(_plot_grp->checkedId()) {
  case _TH1F:
    desc = new Ami::DescTH1F(qPrintable(_title->text()),
			     name,"events",
			     _hist->bins(),_hist->lo(),_hist->hi(), false); 
    break;
  case _vT: 
    desc = new Ami::DescScalar(qPrintable(_title->text()),
			       name);
    break;
  case _vF:
    desc = new Ami::DescProf(qPrintable(_title->text()),
			     qPrintable(_vFeature->expr()),name,
			     _vFeature->bins(),_vFeature->lo(),_vFeature->hi(),"mean");
    break;
  case _vS:
    desc = new Ami::DescScan(qPrintable(_title->text()),
			     qPrintable(_vScan->expr()),"mean",_vScan->bins());
    break;
  default:
    desc = 0;
    break;
  }

  PeakFitPlot* plot = new PeakFitPlot(this,
				      _title->text(),
				      _channel,
				      new Ami::PeakFitPlot(*desc,_baseline->value(),
							   (Ami::PeakFitPlot::Parameter)_quantity));
							   
  _plots.push_back(plot);

  connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));

  emit changed();
}

void PeakFit::remove_plot(QObject* obj)
{
  PeakFitPlot* plot = static_cast<PeakFitPlot*>(obj);
  _plots.remove(plot);

  disconnect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
}

