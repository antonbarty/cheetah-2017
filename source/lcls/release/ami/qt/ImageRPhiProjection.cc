#include "ImageRPhiProjection.hh"

#include "ami/qt/AnnulusCursors.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/ProjectionPlot.hh"
#include "ami/qt/CursorPlot.hh"
#include "ami/qt/RPhiProjectionPlotDesc.hh"
#include "ami/qt/ImageIntegral.hh"
#include "ami/qt/Display.hh"
#include "ami/qt/ImageFrame.hh"
#include "ami/qt/AxisBins.hh"

#include "ami/data/BinMath.hh"
#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/Entry.hh"
#include "ami/data/RPhiProjection.hh"
#include "ami/data/FFT.hh"

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
#include <QtGui/QCheckBox>
#include <QtGui/QMessageBox>
#include <QtGui/QTabWidget>

#include <sys/socket.h>

using namespace Ami::Qt;

enum { PlotProjection, PlotIntegral };

ImageRPhiProjection::ImageRPhiProjection(QWidget*           parent,
					 ChannelDefinition* channels[],
					 unsigned           nchannels, 
					 ImageFrame&        frame) :
  QtPWidget (parent),
  _channels (channels),
  _nchannels(nchannels),
  _channel  (0),
  _frame    (frame),
  _annulus  (new AnnulusCursors(frame)),
  _title    (new QLineEdit("Projection"))
{
  setWindowTitle("Image Projection");
  setAttribute(::Qt::WA_DeleteOnClose, false);

  QComboBox* channelBox = new QComboBox;
  for(unsigned i=0; i<nchannels; i++)
    channelBox->addItem(channels[i]->name());

  QPushButton* plotB  = new QPushButton("Plot");
  QPushButton* closeB = new QPushButton("Close");

  _plot_tab        = new QTabWidget(0);
  _projection_plot = new RPhiProjectionPlotDesc(0, *_annulus);
  _integral_plot   = new ImageIntegral(0);
  _plot_tab->insertTab(PlotProjection,_projection_plot,"Projection");
  _plot_tab->insertTab(PlotIntegral  ,_integral_plot  ,"Integral"); 

  QVBoxLayout* layout = new QVBoxLayout;
  { QGroupBox* channel_box = new QGroupBox("Source Channel");
    QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addWidget(new QLabel("Channel"));
    layout1->addWidget(channelBox);
    layout1->addStretch();
    channel_box->setLayout(layout1);
    layout->addWidget(channel_box); }
  { QGroupBox* boundary_box = new QGroupBox("Region of Interest");
    QVBoxLayout* layout1 = new QVBoxLayout;
    layout1->addWidget(_annulus);
    boundary_box->setLayout(layout1);
    layout->addWidget(boundary_box); }
  { QGroupBox* plot_box = new QGroupBox("Plot");
    QVBoxLayout* layout1 = new QVBoxLayout;
    { QHBoxLayout* layout2 = new QHBoxLayout;
      layout2->addWidget(new QLabel("Title"));
      layout2->addWidget(_title);
      layout1->addLayout(layout2); }
    layout1->addWidget(_plot_tab);
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
  connect(_annulus  , SIGNAL(changed()),      this, SLOT(update_range()));
  connect(plotB     , SIGNAL(clicked()),      this, SLOT(plot()));
  connect(closeB    , SIGNAL(clicked()),      this, SLOT(hide()));
}
  
ImageRPhiProjection::~ImageRPhiProjection()
{
}

void ImageRPhiProjection::save(char*& p) const
{
  XML_insert( p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert( p, "unsigned", "_channel", QtPersistent::insert(p,_channel) );
  XML_insert( p, "QLineEdit", "_title" , QtPersistent::insert(p,_title->text()) );
  XML_insert( p, "QTabWidget", "_plot_tab", QtPersistent::insert(p,_plot_tab->currentIndex()) );

  XML_insert( p, "RPhiProjectionPlotDesc", "_projection_plot", _projection_plot->save(p) );
  XML_insert( p, "ImageIntegral", "_integral_plot", _integral_plot->save(p) );

  XML_insert( p, "AnnulusCursors", "_annulus", _annulus->save(p) );

  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++) {
    XML_insert( p, "ProjectionPlot", "_pplots", (*it)->save(p) );
  }

  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++) {
    XML_insert( p, "CursorPlot", "_cplots", (*it)->save(p) );
  }
}

void ImageRPhiProjection::load(const char*& p)
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
  _pplots.clear();

  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++)
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
  _cplots.clear();

  XML_iterate_open(p,tag)
    if (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_channel")
      _channel = QtPersistent::extract_i(p);
    else if (tag.name == "_title")
      _title->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_plot_tab")
      _plot_tab->setCurrentIndex(QtPersistent::extract_i(p));
    else if (tag.name == "_projection_plot")
      _projection_plot->load(p);
    else if (tag.name == "_integral_plot")
      _integral_plot  ->load(p);
    else if (tag.name == "_annulus")
      _annulus->load(p);
    else if (tag.name == "_pplots") {
      ProjectionPlot* plot = new ProjectionPlot(this, p);
      _pplots.push_back(plot);
      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    }
    else if (tag.name == "_cplots") {
      CursorPlot* plot = new CursorPlot(this, p);
      _cplots.push_back(plot);
      connect(plot, SIGNAL(destroyed(QObject*))  , this, SLOT(remove_plot(QObject*)));
    }
  XML_iterate_close(ImageRPhiProjection,tag);
}

void ImageRPhiProjection::save_plots(const QString& p) const
{
  int i=1;
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    (*it)->save_plots(QString("%1_%2").arg(p).arg(i++));
  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++) {
    QString s = QString("%1_%2.dat").arg(p).arg(i++);
    FILE* f = fopen(qPrintable(s),"w");
    if (f) {
      (*it)->dump(f);
      fclose(f);
    }
  }
}

void ImageRPhiProjection::setVisible(bool v)
{
  if (v)  _frame.add_marker(*_annulus);
  else    _frame.remove_marker(*_annulus);
  QWidget::setVisible(v);
  update_range();
}

void ImageRPhiProjection::configure(char*& p, unsigned input, unsigned& output,
				ChannelDefinition* channels[], int* signatures, unsigned nchannels)
{
  const unsigned maxint=0x40000000;
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    (*it)->configure(p,input,output,channels,signatures,nchannels);
  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++)
    (*it)->configure(p,input,output,channels,signatures,nchannels,
		     AxisBins(0,maxint,maxint),Ami::ConfigureRequest::Analysis);
}

void ImageRPhiProjection::setup_payload(Cds& cds)
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
   (*it)->setup_payload(cds);
  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++)
    (*it)->setup_payload(cds);
}

void ImageRPhiProjection::update()
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    (*it)->update();
  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++)
    (*it)->update();
}

void ImageRPhiProjection::set_channel(int c) 
{ 
  _channel=c; 
}

void ImageRPhiProjection::plot()
{
  double f0 = _annulus->phi0();
  double f1 = _annulus->phi1();
  if (!(f0 < f1)) f1 += 2*M_PI;

  switch(_plot_tab->currentIndex()) {
  case PlotProjection:
    { ProjectionPlot* plot = 
	new ProjectionPlot(this,_title->text(), _channel, 
			   _projection_plot->desc(qPrintable(_title->text())));

      _pplots.push_back(plot);

      connect(plot, SIGNAL(description_changed()), this, SLOT(configure_plot()));
      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
      emit changed();

      break;
    }
  case PlotIntegral:
    {
      DescEntry*  desc = _integral_plot->desc(qPrintable(_title->text()));
      CursorPlot* plot = 
 	new CursorPlot(this, _title->text(), _channel, 
                       new BinMath(*desc,_integral_plot->expression()));
      
      _cplots.push_back(plot);

      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
      emit changed();

      break;
    }
  default:
    break;
  }
}

void ImageRPhiProjection::remove_plot(QObject* obj)
{
  { ProjectionPlot* plot = static_cast<ProjectionPlot*>(obj);
    _pplots.remove(plot); }

  { CursorPlot* plot = static_cast<CursorPlot*>(obj);
    _cplots.remove(plot); }

  disconnect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
}

void ImageRPhiProjection::configure_plot()
{
  emit changed();
}

void ImageRPhiProjection::update_range()
{
  _integral_plot->update_range(_annulus->xcenter(),
                               _annulus->ycenter(),
                               _annulus->r_inner(),
                               _annulus->r_outer(),
                               _annulus->phi0(),
                               _annulus->phi1());
}
