#include "WaveformDisplay.hh"

#include "ami/qt/QtBase.hh"
#include "ami/qt/AxisBins.hh"
#include "ami/qt/AxisControl.hh"
#include "ami/qt/Transform.hh"
#include "ami/qt/Cursors.hh"
#include "ami/qt/Path.hh"
#include "ami/qt/PlotFrame.hh"
#include "ami/qt/PrintAction.hh"
#include "ami/qt/Defaults.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/DescWaveform.hh"
#include "ami/data/Entry.hh"

#include <QtGui/QMenuBar>
#include <QtGui/QActionGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>

#include "qwt_plot_canvas.h"
#include "qwt_plot_grid.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"

#include <sys/uio.h>
#include <time.h>
#include <fstream>

using namespace Ami::Qt;

static const double no_scale[] = {0, 1000};

Ami::Qt::WaveformDisplay::WaveformDisplay() :
  QWidget(0),
  _sem   (Ami::Semaphore::FULL)
{
  _plot = new PlotFrame(this);
  _plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
  _plot->setAxisScaleEngine(QwtPlot::yLeft  , new QwtLinearScaleEngine);

  _grid = new QwtPlotGrid;
  bool gMajor = Defaults::instance()->show_grid();
  _grid->enableX   (gMajor);
  _grid->enableY   (gMajor);
  bool gMinor = Defaults::instance()->show_minor_grid();
  _grid->enableXMin(gMinor);
  _grid->enableYMin(gMinor);
  _grid->setMajPen(QPen(QColor(0x808080)));
  _grid->setMinPen(QPen(QColor(0xc0c0c0)));
  _grid->attach(_plot);

  _xbins = new AxisBins(0,1000,10);
  _xinfo = _xbins;
  _yinfo = 0;

  _xrange = new AxisControl(this,"X",false);
  _yrange = new AxisControl(this,"Y",false);

  QPushButton* xtransB = new QPushButton("X Transform");
  _xtransform = new Transform(this, "X Transform","x");

  QMenuBar* menu_bar = new QMenuBar(this);
  { QMenu* file_menu = new QMenu("File");
    file_menu->addAction("Save image"     , this, SLOT(save_image()));
    file_menu->addAction("Save data"      , this, SLOT(save_data ()));
    file_menu->addAction("Save reference" , this, SLOT(save_reference()));
    file_menu->addSeparator();
    menu_bar->addMenu(file_menu); }
  { QMenu* annotate = new QMenu("Annotate");
    annotate->addAction("Plot Title"           , this, SLOT(set_plot_title()));
    annotate->addAction("Y-axis Title (left)"  , this, SLOT(set_yaxis_title()));
    annotate->addAction("X-axis Title (bottom)", this, SLOT(set_xaxis_title()));
    annotate->addAction("Toggle Grid"          , this, SLOT(toggle_grid()));
    annotate->addAction("Toggle Minor Grid"    , this, SLOT(toggle_minor_grid()));
    menu_bar->addMenu(annotate); }

  QVBoxLayout* layout = new QVBoxLayout;
  { QGroupBox* plotBox = new QGroupBox("Plot");
    QVBoxLayout* layout1 = new QVBoxLayout;
    layout1->addWidget(menu_bar);
    layout1->addWidget(_plot);
    plotBox->setLayout(layout1);
    layout->addWidget(plotBox); }
  { QGridLayout* layout2 = new QGridLayout;
    layout2->addWidget(_xrange,0,0);
    layout2->addWidget(_yrange,0,1);
    { QHBoxLayout* layout3 = new QHBoxLayout;
      layout3->addStretch();
      layout3->addWidget(xtransB);
      layout3->addStretch();
      layout2->addLayout(layout3,1,0); }
    layout->addLayout(layout2); }
  setLayout(layout);

  connect(this   , SIGNAL(redraw()) , _plot      , SLOT(replot()));
  connect(_xrange, SIGNAL(windowChanged()), this , SLOT(xrange_change()));
  connect(_yrange, SIGNAL(windowChanged()), this , SLOT(yrange_change()));
  connect(xtransB, SIGNAL(clicked()), _xtransform, SLOT(show()));
  connect(_xtransform, SIGNAL(changed()), this, SLOT(xtransform_update()));
}

WaveformDisplay::~WaveformDisplay()
{
  delete _xbins;
}

void WaveformDisplay::save(char*& p) const
{
  XML_insert(p, "Transform", "_xtransform", _xtransform->save(p) );
  XML_insert(p, "AxisControl", "_xrange", _xrange->save(p) );
  XML_insert(p, "AxisControl", "_yrange", _yrange->save(p) );
  XML_insert(p, "bool", "_grid_xenabled", QtPersistent::insert(p,_grid->xEnabled()) );
  XML_insert(p, "bool", "_grid_xminenabled", QtPersistent::insert(p,_grid->xMinEnabled()) );
}

void WaveformDisplay::load(const char*& p)
{
  XML_iterate_open(p,tag)

    if (tag.name == "_xtransform")
      _xtransform->load(p);
    else if (tag.name == "_xrange")
      _xrange->load(p);
    else if (tag.name == "_yrange")
      _yrange->load(p);
    else if (tag.name == "_grid_xenabled") {
      bool gMajor = QtPersistent::extract_b(p);
      _grid->enableX   (gMajor);
      _grid->enableY   (gMajor);
    }
    else if (tag.name == "_grid_xminenabled") {
      bool gMinor = QtPersistent::extract_b(p);
      _grid->enableXMin(gMinor);
      _grid->enableYMin(gMinor);
    }

  XML_iterate_close(WaveformDisplay,tag);
}

void WaveformDisplay::save_image()
{
  char time_buffer[32];
  time_t seq_tm = time(NULL);
  strftime(time_buffer,32,"%Y%m%d_%H%M%S",localtime(&seq_tm));

  QString def("ami");
  def += "_";
  def += time_buffer;
  def += ".bmp";
  QString fname =
    QFileDialog::getSaveFileName(this,"Save File As (.bmp,.jpg,.png)",
                                 def,".bmp;.png;.jpg");
  if (!fname.isNull()) {
    QPixmap pixmap(QWidget::size());
    QWidget::render(&pixmap);
    pixmap.toImage().save(fname);
  }
}

void WaveformDisplay::save_plots(const QString& p) const
{
  QString fname = QString("%1.dat").arg(p);
  FILE* f = fopen(qPrintable(fname),"w");
  if (!f)
    QMessageBox::warning(0, "Save data",
			 QString("Error opening %1 for writing").arg(fname));
  else {
    _sem.take();
    for(std::list<QtBase*>::const_iterator it=_curves.begin(); it!=_curves.end(); it++) {
      (*it)->dump(f);
      fprintf(f,"\n");
    }
    _sem.give();
    fclose(f);
  }
}

void WaveformDisplay::save_data()
{
  if (!_curves.size())
    QMessageBox::warning(this, "Save data",
			 QString("No data to save"));
  else {

    char time_buffer[32];
    time_t seq_tm = time(NULL);
    strftime(time_buffer,32,"%Y%m%d_%H%M%S",localtime(&seq_tm));

    QString def = QString("%1/%2.dat").arg(Path::base()).arg(time_buffer);
    QString fname =
      QFileDialog::getSaveFileName(this,"Save File As (.dat)",
				   def,"*.dat");
    if (!fname.isNull()) {
      if (fname.endsWith(".dat"))
	fname.remove(fname.size()-4,4);
      save_plots(fname);
    }
  }
}

void WaveformDisplay::save_reference()
{
  QStringList list;
  _sem.take();
  for(std::list<QtBase*>::const_iterator it=_curves.begin(); it!=_curves.end(); it++) 
    list << (*it)->title();
  _sem.give();
  
  bool ok;
  QString choice = QInputDialog::getItem(this,"Reference Channel","Input",list,0,false,&ok);
  if (!ok) return;
  
  QtBase* ref = 0;
  _sem.take();
  for(std::list<QtBase*>::const_iterator it=_curves.begin(); it!=_curves.end(); it++) 
    if ((*it)->title()==choice) {
      ref = (*it);
      break;
    }
  _sem.give();

  if (ref==0) {
    printf("Reference %s not found\n",qPrintable(choice));
    return;
  }

  FILE* f = Path::saveReferenceFile(ref->title());
  if (f) {
    fwrite(&ref->entry().desc(), ref->entry().desc().size(), 1, f);
    iovec iov;  ref->entry().payload(iov);
    fwrite(iov.iov_base, iov.iov_len, 1, f);
    fclose(f);
  }
}
	  
void WaveformDisplay::set_plot_title()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("Plot Title"), tr("Enter new title:"), 
				       QLineEdit::Normal, _plot->title().text(), &ok);
  if (ok)
    _plot->setTitle(text);
}

void WaveformDisplay::set_xaxis_title()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("X-Axis Title"), tr("Enter new title:"), 
				       QLineEdit::Normal, _plot->axisTitle(QwtPlot::xBottom).text(), &ok);
  if (ok)
    _plot->setAxisTitle(QwtPlot::xBottom,text);
}

void WaveformDisplay::set_yaxis_title()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("Y-Axis Title"), tr("Enter new title:"), 
				       QLineEdit::Normal, _plot->axisTitle(QwtPlot::yLeft).text(), &ok);
  if (ok)
    _plot->setAxisTitle(QwtPlot::yLeft,text);
}

void WaveformDisplay::toggle_grid()
{
  bool gEnable = !_grid->xEnabled();
  _grid->enableX(gEnable);
  _grid->enableY(gEnable);
  emit redraw();
}

void WaveformDisplay::toggle_minor_grid()
{
  bool gEnable = !_grid->xMinEnabled();
  _grid->enableXMin(gEnable);
  _grid->enableYMin(gEnable);
  emit redraw();
}

void WaveformDisplay::prototype(const Ami::DescEntry* e)
{
#define CASETYPE(type)							\
    case Ami::DescEntry::type:						\
      { const Ami::Desc##type& d = *static_cast<const Ami::Desc##type*>(e); \
	_xbins->update(d.xlow(),d.xup(),d.nbins());			\
	break; }						       
  switch(e->type()) {
    CASETYPE(TH1F)
    CASETYPE(Prof)
    CASETYPE(Waveform)
  default:
    break;
  }

  _xrange->update(*_xinfo);
}

void WaveformDisplay::add   (QtBase* b, bool show) 
{
  if (show) {
    _xrange->update(*_xinfo);
    _sem.take();
    _curves.push_back(b);
    _sem.give();
    b->xscale_update();
    b->update();
    b->attach(_plot);
    emit redraw();
  }
  else {
    _hidden.push_back(b);
    b->xscale_update();
    b->update();
    b->attach(_plot);
  }
}

void WaveformDisplay::show(QtBase* b)
{
  for(std::list<QtBase*>::iterator it=_hidden.begin(); it!=_hidden.end(); it++) {
    if ((*it)==b) {
      _sem.take();
      _hidden.remove(b);
      _curves.push_back(b);
      _sem.give();

      b->xscale_update();
      b->update();
      b->attach(_plot);
      
      emit redraw();
      break;
    }
  }
}

void WaveformDisplay::hide(QtBase* b)
{
  _sem.take();
  for(std::list<QtBase*>::iterator it=_curves.begin(); it!=_curves.end(); it++) {
    if ((*it)==b) {
      _curves.remove(b);
      _hidden.push_back(b);
      
      b->attach((QwtPlot*)NULL);
      
      emit redraw();
      break;
    }
  }
  _sem.give();
}

void WaveformDisplay::reset()
{
  _sem.take();
  _curves.merge(_hidden);

  for(std::list<QtBase*>::iterator it=_curves.begin(); it!=_curves.end(); it++) {
    delete (*it);
  }
  _curves.clear();
  _sem.give();
}

void WaveformDisplay::update()
{
  _sem.take();
  for(std::list<QtBase*>::iterator it=_curves.begin(); it!=_curves.end(); it++)
    (*it)->update();
  _sem.give();

  emit redraw();
}

void WaveformDisplay::xtransform_update()
{
  _sem.take();
  for(std::list<QtBase*>::iterator it=_curves.begin(); it!=_curves.end(); it++)
    (*it)->xscale_update();
  _sem.give();

  if (_curves.size())
    _xrange->update(*_xinfo);

  if (_xrange->isAuto()) {
  }

  emit redraw();
}

void WaveformDisplay::xrange_change()
{
  if (_xrange->isAuto()) 
    _plot->setAxisAutoScale  (QwtPlot::xBottom);
  else
    _plot->setAxisScale      (QwtPlot::xBottom, _xrange->loEdge(), _xrange->hiEdge());

  if (_xrange->isLog())
    _plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
  else
    _plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
  emit redraw();
}

void WaveformDisplay::yrange_change()
{
  if (_yrange->isAuto()) 
    _plot->setAxisAutoScale  (QwtPlot::yLeft);
  else
    _plot->setAxisScale      (QwtPlot::yLeft, _yrange->loEdge(), _yrange->hiEdge());

  if (_yrange->isLog())
    _plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
  else
    _plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
  emit redraw();
}

const Ami::AbsTransform& WaveformDisplay::xtransform() const { return *_xtransform; }

const std::list<QtBase*> WaveformDisplay::plots() const { return _curves; }

const AxisInfo& WaveformDisplay::xinfo() const { return *_xinfo; }

PlotFrame* WaveformDisplay::plot() const { return _plot; }


