#include "ImageXYProjection.hh"

#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/RectangleCursors.hh"
#include "ami/qt/ProjectionPlot.hh"
#include "ami/qt/CursorPlot.hh"
#include "ami/qt/ZoomPlot.hh"
#include "ami/qt/XYHistogramPlotDesc.hh"
#include "ami/qt/XYProjectionPlotDesc.hh"
#include "ami/qt/ScalarPlotDesc.hh"
#include "ami/qt/ImageIntegral.hh"
#include "ami/qt/Display.hh"
#include "ami/qt/ImageFrame.hh"
#include "ami/qt/AxisBins.hh"

#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/Entry.hh"
#include "ami/data/BinMath.hh"
#include "ami/data/XYHistogram.hh"
#include "ami/data/XYProjection.hh"

#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QComboBox>
#include <QtGui/QMessageBox>
#include <QtGui/QTabWidget>

#include <sys/socket.h>

using namespace Ami::Qt;

enum { PlotHistogram, PlotProjection, PlotIntegral };

ImageXYProjection::ImageXYProjection(QWidget*           parent,
				     ChannelDefinition* channels[],
				     unsigned           nchannels, 
				     ImageFrame&        frame) :
  QtPWidget (parent),
  _channels (channels),
  _nchannels(nchannels),
  _channel  (0),
  _frame    (frame),
  _title    (new QLineEdit("Projection"))
{
  _rectangle = new RectangleCursors(_frame);

  setWindowTitle("Image Projection");
  setAttribute(::Qt::WA_DeleteOnClose, false);

  QComboBox* channelBox = new QComboBox;
  for(unsigned i=0; i<nchannels; i++)
    channelBox->addItem(channels[i]->name());

  QPushButton* zoomB  = new QPushButton("Zoom");
  QPushButton* plotB  = new QPushButton("Plot");
  QPushButton* closeB = new QPushButton("Close");

  _plot_tab        = new QTabWidget(0);
  _histogram_plot  = new XYHistogramPlotDesc (0, *_rectangle);
  _projection_plot = new XYProjectionPlotDesc(0, *_rectangle);
  //  _integral_plot   = new ScalarPlotDesc(0);
  _integral_plot   = new ImageIntegral(0);
  _plot_tab->insertTab(PlotHistogram ,_histogram_plot ,"Histogram");
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
  { QGroupBox* locations_box = new QGroupBox("Define Boundaries");
    locations_box->setToolTip("Define projection boundaries.");
    QVBoxLayout* layout2 = new QVBoxLayout;
    layout2->addWidget(_rectangle);
    locations_box->setLayout(layout2);
    layout->addWidget(locations_box); }
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
    layout1->addWidget(zoomB);
    layout1->addWidget(plotB);
    layout1->addWidget(closeB);
    layout1->addStretch();
    layout->addLayout(layout1); }

  setLayout(layout);

  connect(channelBox, SIGNAL(activated(int)), this, SLOT(set_channel(int)));
  connect(_rectangle, SIGNAL(changed()),      this, SLOT(update_range()));
  connect(plotB     , SIGNAL(clicked()),      this, SLOT(plot()));
  connect(zoomB     , SIGNAL(clicked()),      this, SLOT(zoom()));
  connect(closeB    , SIGNAL(clicked()),      this, SLOT(hide()));
}
  
ImageXYProjection::~ImageXYProjection()
{
}

void ImageXYProjection::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert(p, "int", "_channel", QtPersistent::insert(p,_channel) );
  XML_insert(p, "QLineEdit", "_title", QtPersistent::insert(p,_title->text()) );
  XML_insert(p, "QComboBox", "_plot_tab", QtPersistent::insert(p,_plot_tab->currentIndex()) );
  
  XML_insert(p, "XYHistogramPlotDesc", "_histogram_plot", _histogram_plot ->save(p) );
  XML_insert(p, "XYProjectionPlotDesc", "_projection_plot", _projection_plot->save(p) );
  XML_insert(p, "ImageIntegral", "_integral_plot", _integral_plot  ->save(p) );

  XML_insert(p, "RectangleCursors", "_rectangle", _rectangle->save(p) );

  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++) {
    XML_insert(p, "ProjectionPlot", "_pplots", (*it)->save(p) );
  }

  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++) {
    XML_insert(p, "CursorPlot", "_cplots", (*it)->save(p) );
  }

  for(std::list<ZoomPlot*>::const_iterator it=_zplots.begin(); it!=_zplots.end(); it++) {
    XML_insert(p, "ZoomPlot", "_zplots", (*it)->save(p) );
  }
}

void ImageXYProjection::load(const char*& p) 
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
  _pplots.clear();

  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++)
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
  _cplots.clear();

  for(std::list<ZoomPlot*>::const_iterator it=_zplots.begin(); it!=_zplots.end(); it++)
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
  _zplots.clear();

  XML_iterate_open(p,tag)
   if (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_channel")
      _channel = QtPersistent::extract_i(p);
    else if (tag.name == "_title")
      _title->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_plot_tab")
      _plot_tab->setCurrentIndex(QtPersistent::extract_i(p));
    else if (tag.name == "_histogram_plot")
      _histogram_plot ->load(p);
    else if (tag.name == "_projection_plot")
      _projection_plot->load(p);
    else if (tag.name == "_integral_plot")
      _integral_plot  ->load(p);
    else if (tag.name == "_rectangle")
      _rectangle->load(p);
    else if (tag.name == "_pplots") {
      ProjectionPlot* plot = new ProjectionPlot(this, p);
      _pplots.push_back(plot);
      connect(plot, SIGNAL(description_changed()), this, SLOT(configure_plot()));
      connect(plot, SIGNAL(destroyed(QObject*))  , this, SLOT(remove_plot(QObject*)));
    }
    else if (tag.name == "_cplots") {
      CursorPlot* plot = new CursorPlot(this, p);
      _cplots.push_back(plot);
      connect(plot, SIGNAL(destroyed(QObject*))  , this, SLOT(remove_plot(QObject*)));
    }
    else if (tag.name == "_zplots") {
    ZoomPlot* plot = new ZoomPlot(this, p);
    _zplots.push_back(plot);
    connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    }
  XML_iterate_close(ImageXYProjection,tag);
}

void ImageXYProjection::save_plots(const QString& p) const
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

void ImageXYProjection::setVisible(bool v)
{
  if (v)    _frame.add_marker(*_rectangle);
  else      _frame.remove_marker(*_rectangle);
  QWidget::setVisible(v);
  update_range();
}

void ImageXYProjection::configure(char*& p, unsigned input, unsigned& output,
				  ChannelDefinition* channels[], int* signatures, unsigned nchannels)
{
  const unsigned maxpixels=1024;
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    (*it)->configure(p,input,output,channels,signatures,nchannels);
  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++)
    (*it)->configure(p,input,output,channels,signatures,nchannels,
		     AxisBins(0,maxpixels,maxpixels),Ami::ConfigureRequest::Analysis);
  for(std::list<ZoomPlot*>::const_iterator it=_zplots.begin(); it!=_zplots.end(); it++)
    (*it)->configure(p,input,output,channels,signatures,nchannels);
}

void ImageXYProjection::setup_payload(Cds& cds)
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
   (*it)->setup_payload(cds);
  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++)
    (*it)->setup_payload(cds);
  for(std::list<ZoomPlot*>::const_iterator it=_zplots.begin(); it!=_zplots.end(); it++)
   (*it)->setup_payload(cds);
}

void ImageXYProjection::update()
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    (*it)->update();
  for(std::list<CursorPlot*>::const_iterator it=_cplots.begin(); it!=_cplots.end(); it++)
    (*it)->update();
  for(std::list<ZoomPlot*>::const_iterator it=_zplots.begin(); it!=_zplots.end(); it++)
    (*it)->update();
}

void ImageXYProjection::set_channel(int c) 
{ 
  _channel=c; 
}

void ImageXYProjection::plot()
{
  switch(_plot_tab->currentIndex()) {
  case PlotHistogram:
  case PlotProjection:
    { AbsOperator* op = _plot_tab->currentIndex()==PlotHistogram ?
        static_cast<AbsOperator*>(_histogram_plot ->desc(qPrintable(_title->text()))) :
        static_cast<AbsOperator*>(_projection_plot->desc(qPrintable(_title->text())));
      ProjectionPlot* plot = 
	new ProjectionPlot(this,_title->text(), _channel, op);
                           
      
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
      delete desc;
      
      _cplots.push_back(plot);

      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
      emit changed();

      break;
    }
  default:
    return;
  }

}

void ImageXYProjection::zoom()
{
  ZoomPlot* plot = new ZoomPlot(this,
				_channels[_channel]->name(),
				_channel,
				unsigned(_rectangle->xlo()),
				unsigned(_rectangle->ylo()),
                                unsigned(_rectangle->xhi()),
                                unsigned(_rectangle->yhi()));
  _zplots.push_back(plot);

  connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));

  emit changed();
}

void ImageXYProjection::remove_plot(QObject* obj)
{
  printf("Removing plot %p\n",obj);
  { ProjectionPlot* plot = static_cast<ProjectionPlot*>(obj);
    _pplots.remove(plot); }

  { CursorPlot* plot = static_cast<CursorPlot*>(obj);
    _cplots.remove(plot); }

  { ZoomPlot* plot = static_cast<ZoomPlot*>(obj);
    _zplots.remove(plot); }

  disconnect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));

  emit changed();
}

void ImageXYProjection::configure_plot()
{
  emit changed();
}

void ImageXYProjection::update_range()
{
  _integral_plot->update_range(_rectangle->ixlo(),
                               _rectangle->iylo(),
                               _rectangle->ixhi(),
                               _rectangle->iyhi());
}
