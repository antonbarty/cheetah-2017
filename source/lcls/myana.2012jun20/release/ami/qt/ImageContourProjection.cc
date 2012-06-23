#include "ImageContourProjection.hh"

#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/RectangleCursors.hh"
#include "ami/qt/ProjectionPlot.hh"
#include "ami/qt/Display.hh"
#include "ami/qt/ImageFrame.hh"
#include "ami/qt/Contour.hh"

#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/Entry.hh"
#include "ami/data/ContourProjection.hh"

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

enum { XAxis, YAxis };

ImageContourProjection::ImageContourProjection(QWidget*           parent,
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
  //  _contour   = new Contour("X","f(X)",frame,Ami::ContourProjection::Y,*_rectangle);
  _contour   = new Contour("Y","f(Y)",Ami::ContourProjection::X,frame,*_rectangle);

  setWindowTitle("Contour Projection");
  setAttribute(::Qt::WA_DeleteOnClose, false);

  QComboBox* channelBox = new QComboBox;
  for(unsigned i=0; i<nchannels; i++)
    channelBox->addItem(channels[i]->name());

  QPushButton* plotB  = new QPushButton("Plot");
  QPushButton* closeB = new QPushButton("Close");

  QRadioButton* xaxisB = new QRadioButton("X\' Axis : X' = X - f(Y)");
  QRadioButton* yaxisB = new QRadioButton("Y\' Axis : Y' = Y - f(X)");
  _axis = new QButtonGroup;
  _axis->addButton(xaxisB,XAxis);
  _axis->addButton(yaxisB,YAxis);
  //  yaxisB->setChecked(true);
  xaxisB->setChecked(true);
  connect(xaxisB, SIGNAL(toggled(bool)), this, SLOT(use_xaxis(bool)));

  QRadioButton* sumB  = new QRadioButton("sum");
  QRadioButton* meanB = new QRadioButton("mean");
  _norm = new QButtonGroup;
  _norm->addButton(sumB ,0);
  _norm->addButton(meanB,1);
  sumB->setChecked(true);

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
    { QHBoxLayout* layout2 = new QHBoxLayout;
      layout2->addWidget(new QLabel("Project"));
      { QVBoxLayout* layout3 = new QVBoxLayout;
	layout3->addWidget(sumB);
	layout3->addWidget(meanB);
	layout2->addLayout(layout3); }
      layout2->addWidget(new QLabel("onto"));
      { QVBoxLayout* layout3 = new QVBoxLayout;
 	layout3->addWidget(xaxisB);
 	layout3->addWidget(yaxisB);
 	layout2->addLayout(layout3); }
      layout2->addWidget(new QLabel("axis"));
      layout2->addStretch();
      layout1->addLayout(layout2); }
    layout1->addWidget(_contour);
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
  
ImageContourProjection::~ImageContourProjection()
{
}

void ImageContourProjection::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert(p, "unsigned", "_channel", QtPersistent::insert(p,_channel) );
  XML_insert(p, "QLineEdit", "_title", QtPersistent::insert(p,_title->text()) );
  XML_insert(p, "QButtonGroup", "_axis", QtPersistent::insert(p,_axis ->checkedId()) );
  XML_insert(p, "QButtonGroup", "_norm", QtPersistent::insert(p,_norm ->checkedId()) );
  XML_insert(p, "RectangleCursors", "_rectangle", _rectangle->save(p) );
  XML_insert(p, "Contour", "_contour", _contour->save(p) );

  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++) {
    XML_insert(p, "ProjectionPlot", "_pplots", (*it)->save(p) );
  }
}

void ImageContourProjection::load(const char*& p) 
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    disconnect(*it, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
  _pplots.clear();

  XML_iterate_open(p,tag)
    if      (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_channel")
      _channel = QtPersistent::extract_i(p);
    else if (tag.name == "_title")
      _title->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_axis")
      _axis ->button(QtPersistent::extract_i(p))->setChecked(true);
    else if (tag.name == "_norm")
      _norm ->button(QtPersistent::extract_i(p))->setChecked(true);
    else if (tag.name == "_rectangle")
      _rectangle->load(p);
    else if (tag.name == "_contour")
      _contour->load(p);
    else if (tag.name == "_pplots") {
      ProjectionPlot* plot = new ProjectionPlot(this, p);
      _pplots.push_back(plot);
      connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
    }
  XML_iterate_close(ImageContourProjection,tag);
}

void ImageContourProjection::save_plots(const QString& p) const
{
  int i=1;
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    (*it)->save_plots(QString("%1_%2").arg(p).arg(i++));
}

void ImageContourProjection::setVisible(bool v)
{
  if (v) {
    _frame.add_marker(*_rectangle);
    _frame.add_marker(*_contour);
  }
  else {
    _frame.remove_marker(*_rectangle);
    _frame.remove_marker(*_contour);
  }
  QWidget::setVisible(v);
}

void ImageContourProjection::configure(char*& p, unsigned input, unsigned& output,
				       ChannelDefinition* channels[], int* signatures, unsigned nchannels)
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    (*it)->configure(p,input,output,channels,signatures,nchannels);
}

void ImageContourProjection::setup_payload(Cds& cds)
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
   (*it)->setup_payload(cds);
}

void ImageContourProjection::update()
{
  for(std::list<ProjectionPlot*>::const_iterator it=_pplots.begin(); it!=_pplots.end(); it++)
    (*it)->update();
}

void ImageContourProjection::set_channel(int c) 
{ 
  _channel=c; 
}

void ImageContourProjection::plot()
{
  Ami::ContourProjection* proj;

  if (_axis->checkedId()==0) { // X
    Ami::Contour f = _contour->value();
    double xmin, xmax;
    f.extremes(_rectangle->ylo(), _rectangle->yhi(),
	       xmin, xmax);

    double x0 = (_rectangle->xlo() - xmin);
    double x1 = (_rectangle->xhi() - xmax+1);

    double xlo = (_rectangle->xlo());
    double xhi = (_rectangle->xhi());
    double ylo = (_rectangle->ylo());
    double yhi = (_rectangle->yhi());

    const AxisInfo& xinfo = *_frame.xinfo();
    int nbx = xinfo.tick(x1-x0)+1;

    if (_norm->checkedId()==0) {
      Ami::DescTH1F desc(qPrintable(_title->text()),
 			 "pixel", "sum",
			 nbx, x0, x1);
      proj = new Ami::ContourProjection(desc, f,
 					Ami::ContourProjection::X, 
 					xlo, xhi, ylo, yhi);
    }
    else {
      Ami::DescProf desc(qPrintable(_title->text()),
			 "pixel", "mean",
			 nbx, x0, x1, "");
      proj = new Ami::ContourProjection(desc, f,
 					Ami::ContourProjection::X, 
 					xlo, xhi, ylo, yhi);
    }
  }
  else { // Y
    Ami::Contour f = _contour->value();
    double ymin, ymax;
    f.extremes(_rectangle->xlo(), _rectangle->xhi(),
	       ymin, ymax);

    double y0 = (_rectangle->ylo() - ymin);
    double y1 = (_rectangle->yhi() - ymax+1);

    double xlo = (_rectangle->xlo());
    double xhi = (_rectangle->xhi());
    double ylo = (_rectangle->ylo());
    double yhi = (_rectangle->yhi());

    const AxisInfo& yinfo = *_frame.yinfo();
    int nby = yinfo.tick(y1-y0)+1;

    if (_norm->checkedId()==0) {
      Ami::DescTH1F desc(qPrintable(_title->text()),
			 "pixel", "sum",
			 nby, y0, y1);
      proj = new Ami::ContourProjection(desc, f,
					Ami::ContourProjection::Y, 
					xlo, xhi, ylo, yhi);
    }
    else {
      Ami::DescProf desc(qPrintable(_title->text()),
			 "pixel", "mean",
			 nby, y0, y1, "");
      proj = new Ami::ContourProjection(desc, f,
					Ami::ContourProjection::Y, 
					xlo, xhi, ylo, yhi);
    }
  }
  
  ProjectionPlot* plot = new ProjectionPlot(this,_title->text(), _channel, proj);

  _pplots.push_back(plot);

  connect(plot, SIGNAL(description_changed()), this, SLOT(configure_plot()));
  connect(plot, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));

  emit changed();
}

void ImageContourProjection::remove_plot(QObject* obj)
{
  { ProjectionPlot* plot = static_cast<ProjectionPlot*>(obj);
    _pplots.remove(plot); }

  disconnect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(remove_plot(QObject*)));
}

void ImageContourProjection::configure_plot()
{
  emit changed();
}

void ImageContourProjection::use_xaxis(bool v)
{
  if (v) _contour->setup("Y","f(Y)",Ami::ContourProjection::X);
  else   _contour->setup("X","f(X)",Ami::ContourProjection::Y);
}
