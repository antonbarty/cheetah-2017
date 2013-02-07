#include "PeakFit.hh"

#include "ami/qt/DescTH1F.hh"
#include "ami/qt/DescProf.hh"
#include "ami/qt/DescScan.hh"
#include "ami/qt/DescChart.hh"
#include "ami/qt/AxisInfo.hh"
#include "ami/qt/AxisBins.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/EdgeCursor.hh"
#include "ami/qt/PeakFitPlot.hh"
#include "ami/qt/PeakFitPost.hh"
#include "ami/qt/WaveformDisplay.hh"
#include "ami/qt/Calculator.hh"
#include "ami/qt/PlotFrame.hh"
#include "ami/qt/FeatureRegistry.hh"
#include "ami/qt/CursorsX.hh"
#include "ami/qt/CursorDefinition.hh"

#include "ami/data/DescScalar.hh"
#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/DescScan.hh"
#include "ami/data/DescCache.hh"
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
  _clayout  (new QVBoxLayout)
{
  _names << "a" << "b" << "c" << "d" << "f" << "g" << "h" << "i" << "j" << "k";

  setWindowTitle("PeakFit Plot");
  setAttribute(::Qt::WA_DeleteOnClose, false);

  QComboBox* channelBox = new QComboBox;
  for(unsigned i=0; i<nchannels; i++)
    channelBox->addItem(channels[i]->name());

  _baseline  = new EdgeCursor(QString(""), *_frame.plot());
  QString bl("baseline");
  _baseline->setName(bl);

  QStringList q;
  for(unsigned k=0; k<Ami::PeakFitPlot::NumberOf; k++)
    q << Ami::PeakFitPlot::name((Ami::PeakFitPlot::Parameter)k);
  QComboBox* qtyBox = new QComboBox;
  qtyBox->addItems(q);

  _title    = new QLineEdit("Peak plot");
  QPushButton* addPostB = new QPushButton("Post");

  _hist   = new DescTH1F (bold(Sum (1dH)));
  _vTime  = new DescChart(bold(Mean v Time));
  _vFeature = new DescProf (bold(Mean v Var) , &FeatureRegistry::instance());
  _vScan    = new DescScan (bold(Mean v Scan));

  _plot_grp = new QButtonGroup;
  _plot_grp->addButton(_hist    ->button(),_TH1F);
  _plot_grp->addButton(_vTime   ->button(),_vT);
  _plot_grp->addButton(_vFeature->button(),_vF);
  _plot_grp->addButton(_vScan   ->button(),_vS);
  _hist->button()->setChecked(true);

  _const_bl  = new QRadioButton("constant baseline");
  _linear_bl = new QRadioButton("linear baseline  ");
  _lvalue = new CursorLocation;
  QPushButton *lgrabB  = new QPushButton("Grab");

  _base_grp = new QButtonGroup;
  _base_grp->addButton(_const_bl, 0);
  _base_grp->addButton(_linear_bl, 0);
  _const_bl->setChecked(true);

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
    QVBoxLayout* layout1 = new QVBoxLayout;
    { QGridLayout *layout2 = new QGridLayout;
      layout2->addWidget(_const_bl,  0, 0);
      layout2->addWidget(_baseline,  0, 1);
      layout2->addWidget(_linear_bl, 1, 0);
      { QHBoxLayout* layout3 = new QHBoxLayout;
        layout3->addWidget(_lvalue);
        layout3->addWidget(lgrabB);
        layout2->addLayout(layout3, 1, 1, ::Qt::AlignRight); }
      layout1->addLayout(layout2); }
    layout1->addLayout(_clayout);
    { QHBoxLayout* layout2 = new QHBoxLayout;
      layout2->addWidget(new QLabel("Quantity"));
      layout2->addWidget(qtyBox);
      layout1->addLayout(layout2); }
    locations_box->setLayout(layout1);
    layout->addWidget(locations_box); }
  { QGroupBox* plot_box = new QGroupBox("Plot");
    QVBoxLayout* layout1 = new QVBoxLayout;
    { QHBoxLayout* layout2 = new QHBoxLayout;
      layout2->addWidget(new QLabel("Title"));
      layout2->addWidget(_title);
      layout2->addStretch();
      layout2->addWidget(addPostB);
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

  connect(lgrabB    , SIGNAL(clicked()),      this, SLOT(grab_cursorx()));
  connect(_lvalue   , SIGNAL(returnPressed()),this, SLOT(add_cursor()));
  connect(this      , SIGNAL(grabbed()),      this, SLOT(add_cursor()));

  connect(addPostB  , SIGNAL(clicked()),      this, SLOT(add_post()));

  set_quantity(0);
}
  
PeakFit::~PeakFit()
{
  for(std::list<PeakFitPost*>::const_iterator it=_posts.begin(); it!=_posts.end(); it++) {
    delete *it;
  }
  _posts.clear();
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
  XML_insert(p, "QRadioButton", "_const_bl", QtPersistent::insert(p,_const_bl->isChecked()));

  for(std::list<CursorDefinition*>::const_iterator it=_cursors.begin(); it!=_cursors.end(); it++) {
    XML_insert(p, "CursorDefinition", "_cursors", (*it)->save(p) );
  }

  for(std::list<PeakFitPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    XML_insert(p, "PeakFitPlot", "_plots", (*it)->save(p) );
  }

  for(std::list<PeakFitPost*>::const_iterator it=_posts.begin(); it!=_posts.end(); it++) {
    XML_insert(p, "PeakFitPost", "_posts", (*it)->save(p) );
  }
}

void PeakFit::load(const char*& p)
{
  for(std::list<CursorDefinition*>::const_iterator it=_cursors.begin(); it!=_cursors.end(); it++) {
    _names.push_back((*it)->name());
    delete *it;
  }
  _cursors.clear();

  for(std::list<PeakFitPlot*>::const_iterator it=_plots.begin(); it!=_plots.end(); it++) {
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    delete *it;
  }
  _plots.clear();

  for(std::list<PeakFitPost*>::const_iterator it=_posts.begin(); it!=_posts.end(); it++) {
    delete *it;
  }
  _posts.clear();

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
    else if (tag.name == "_const_bl") {
        if (QtPersistent::extract_b(p))
            _const_bl->setChecked(true);
        else
            _linear_bl->setChecked(true);
    }
    else if (tag.name == "_cursors") {
      CursorDefinition* d = new CursorDefinition(p, *this, _frame.plot());
      _cursors.push_back(d);
      _clayout->addWidget(d);
      _names.removeAll(d->name());
      printf("Added cursor %s at %g\n",qPrintable(d->name()), d->location());
    }    
    else if (tag.name == "_plots") {
      PeakFitPlot* plot = new PeakFitPlot(this, p);
      _plots.push_back(plot);
      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    }
    else if (tag.name == "_posts") {
      PeakFitPost* post = new PeakFitPost(p);
      _posts.push_back(post);
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
  for(std::list<PeakFitPost*>::const_iterator it=_posts.begin(); it!=_posts.end(); it++)
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
  if (!_const_bl->isChecked() && !_cursors.size())  /* Can't plot if no baseline! */
      return;

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


  Ami::PeakFitPlot *plotter;
  if (_const_bl->isChecked()) {
      plotter = new Ami::PeakFitPlot(*desc, _baseline->value(), (Ami::PeakFitPlot::Parameter)_quantity);
  } else {
      int bins[MAX_BINS], i = 0;
      for (std::list<CursorDefinition*>::const_iterator it=_cursors.begin(); it!=_cursors.end() && i < MAX_BINS; i++, it++) {
          bins[i] = _frame.xinfo().tick((*it)->location());
#if 0
          const AxisBins *xi = dynamic_cast<const AxisBins *>(&_frame.xinfo());
          printf("PeakFit: (%d,%d,%d) %lg -> bin %d\n", 
                 _frame.xinfo().lo(), _frame.xinfo().hi(), xi ? xi->ticks() : -1,
                 (*it)->location(), bins[i]);
#endif
      }
      plotter = new Ami::PeakFitPlot(*desc, i, bins, (Ami::PeakFitPlot::Parameter)_quantity);
  }
  PeakFitPlot* plot = new PeakFitPlot(this,
				      _title->text(),
				      _channel,
                                      plotter);
							   
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

void PeakFit::add_post()
{
  //
  //  Check that a post variable by the same name doesn't already exist
  //

  //
  //  Add to the list of post variables
  //
  const char* name = Ami::PeakFitPlot::name((Ami::PeakFitPlot::Parameter)_quantity);
  Ami::DescCache* desc = new Ami::DescCache(name, qPrintable(_title->text()), Ami::PostAnalysis);
  Ami::PeakFitPlot *plotter;
  if (_const_bl->isChecked()) {
      plotter = new Ami::PeakFitPlot(*desc, _baseline->value(), (Ami::PeakFitPlot::Parameter)_quantity);
  } else {
      int bins[MAX_BINS], i = 0;
      for (std::list<CursorDefinition*>::const_iterator it=_cursors.begin(); it!=_cursors.end() && i < MAX_BINS; i++, it++)
          bins[i] = _frame.xinfo().tick((*it)->location());
      plotter = new Ami::PeakFitPlot(*desc,i, bins, (Ami::PeakFitPlot::Parameter)_quantity);
  }
  PeakFitPost* post = new PeakFitPost(_channel, plotter);
							   
  _posts.push_back(post);

  emit changed();
}

void PeakFit::grab_cursorx() { _frame.plot()->set_cursor_input(this); }

void PeakFit::add_cursor()
{
  if (_names.size()) {
    CursorDefinition* d = new CursorDefinition(_names.takeFirst(),
					       _lvalue->value(),
					       *this,
					       _frame.plot());
    _cursors.push_back(d);
    _clayout->addWidget(d);
  }
  else {
    QMessageBox::critical(this,tr("Add Cursor"),tr("Too many cursors in use"));
  }
}

void PeakFit::remove(CursorDefinition& c)
{
  _names.push_back(c.name());
  _cursors.remove(&c);
  delete &c;
}

void PeakFit::mousePressEvent(double x, double y)
{
  _frame.plot()->set_cursor_input(0);
  _lvalue->setText(QString::number(x));
  emit grabbed();
}

void PeakFit::mouseMoveEvent   (double,double) {}
void PeakFit::mouseReleaseEvent(double,double) {}
