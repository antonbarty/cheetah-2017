#include "ImageDisplay.hh"

#include "ami/qt/QtBase.hh"
#include "ami/qt/AxisArray.hh"
#include "ami/qt/ImageGridScale.hh"
#include "ami/qt/ImageColorControl.hh"
#include "ami/qt/Transform.hh"
#include "ami/qt/Cursors.hh"
#include "ami/qt/Path.hh"
#include "ami/qt/ImageFrame.hh"
#include "ami/qt/NullTransform.hh"
#include "ami/qt/PrintAction.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"

#include "pdsdata/xtc/ClockTime.hh"

#include <QtGui/QMenuBar>
#include <QtGui/QLabel>
#include <QtGui/QActionGroup>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>

#include <sys/uio.h>
#include <time.h>
#include <fstream>

using namespace Ami::Qt;

static const double no_scale[] = {0, 1000};
static NullTransform nullTransform;

Ami::Qt::ImageDisplay::ImageDisplay(bool grab) :
  QWidget(0),
  _sem   (Ami::Semaphore::FULL)
{
  _zrange  = new ImageColorControl(this,"Z");
  _plot    = new ImageFrame(this,*_zrange);
  _units   = new ImageGridScale(*_plot, grab);

  _layout();
}

void Ami::Qt::ImageDisplay::_layout()
{
  QMenuBar* menu_bar = new QMenuBar(this);
  {
    QMenu* file_menu = new QMenu("File");
    file_menu->addAction("Save image"     , this, SLOT(save_image()));
    file_menu->addAction("Save data"      , this, SLOT(save_data ()));
    file_menu->addAction("Save reference" , this, SLOT(save_reference()));
    file_menu->addSeparator();
    menu_bar->addMenu(file_menu);
    menu_bar->addAction(new PrintAction(*this));
  }

  QVBoxLayout* layout = new QVBoxLayout;
  layout->setSpacing(1);
  { QGroupBox* plotBox = new QGroupBox("Plot");
    QVBoxLayout* layout1 = new QVBoxLayout;
    { QHBoxLayout* hl = new QHBoxLayout;
      hl->addWidget(menu_bar);
      hl->addWidget(_time_display=new QLabel("Time: Seconds . nseconds"));
      layout1->addLayout(hl); }
    layout1->addWidget(_plot);
    plotBox->setLayout(layout1);
    layout->addWidget(plotBox); }
  { QHBoxLayout* layout2 = new QHBoxLayout;
    layout2->addWidget(_units );
    layout2->addWidget(_zrange);
    layout->addLayout(layout2); }
  layout->addStretch();
  setLayout(layout);

  connect(this   , SIGNAL(redraw()) , _plot      , SLOT(replot()));
  connect(this   , SIGNAL(redraw()) , this       , SLOT(update_timedisplay()));
}

Ami::Qt::ImageDisplay::~ImageDisplay()
{
}

void ImageDisplay::save(char*& p) const
{
  XML_insert( p, "ImageColorControl", "_zrange", _zrange->save(p) );
}

void ImageDisplay::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "_zrange")
      _zrange->load(p);
  XML_iterate_close(ImageDisplay,tag);
}

void ImageDisplay::save_plots(const QString& p) const
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

const ImageColorControl& ImageDisplay::control() const { return *_zrange; }

ImageGridScale& ImageDisplay::grid_scale() { return *_units; }

void Ami::Qt::ImageDisplay::save_image()
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

void Ami::Qt::ImageDisplay::save_data()
{
  if (!_curves.size())
    QMessageBox::warning(this, "Save data",
			 QString("No data to save"));
  else {

    char time_buffer[32];
    time_t seq_tm = time(NULL);
    strftime(time_buffer,32,"%Y%m%d_%H%M%S",localtime(&seq_tm));

    QString def("ami");
    def += "_";
    def += time_buffer;
    def += ".dat";
    QString fname =
      QFileDialog::getSaveFileName(this,"Save File As (.dat)",
				   def,".dat");
    if (!fname.isNull()) {
      FILE* f = fopen(qPrintable(fname),"w");
      if (!f)
	QMessageBox::warning(this, "Save data",
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
  }
}

void Ami::Qt::ImageDisplay::save_reference()
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
	  
void Ami::Qt::ImageDisplay::add   (QtBase* b, bool show) 
{
  if (show) {
    _sem.take();
    _curves.push_back(b);
    _sem.give();
    b->xscale_update();
    b->update();
    b->attach(_plot);
  }
  else {
    _sem.take();
    _hidden.push_back(b);
    _sem.give();
  }

  emit redraw();
}

void Ami::Qt::ImageDisplay::show(QtBase* b)
{
  _sem.take();
  for(std::list<QtBase*>::iterator it=_hidden.begin(); it!=_hidden.end(); it++) {
    if ((*it)==b) {
      _hidden.remove(b);
      _curves.push_back(b);

      b->xscale_update();
      b->update();
      b->attach(_plot);
      
      emit redraw();
      break;
    }
  }
  _sem.give();
}

void Ami::Qt::ImageDisplay::hide(QtBase* b)
{
  _sem.take();
  for(std::list<QtBase*>::iterator it=_curves.begin(); it!=_curves.end(); it++) {
    if ((*it)==b) {
      _curves.remove(b);
      _hidden.push_back(b);
      break;
    }
  }
  _sem.give();
}

void Ami::Qt::ImageDisplay::reset()
{
  _plot->attach(NULL);

  _sem.take();
  _curves.merge(_hidden);
  for(std::list<QtBase*>::iterator it=_curves.begin(); it!=_curves.end(); it++)
    delete (*it);
  _curves.clear();
  _sem.give();
}

void Ami::Qt::ImageDisplay::update()
{
  _sem.take();
  for(std::list<QtBase*>::iterator it=_curves.begin(); it!=_curves.end(); it++)
    (*it)->update();
  _sem.give();

  emit redraw();
}

const Ami::AbsTransform& Ami::Qt::ImageDisplay::xtransform() const { return nullTransform; }

ImageFrame* Ami::Qt::ImageDisplay::plot() const { return _plot; }

void Ami::Qt::ImageDisplay::update_timedisplay()
{
  const Pds::ClockTime& time = _plot->time();
  _time_display->setText(QString("Time:%1.%2")
                         .arg(QString::number(time.seconds()))
                         .arg(QString::number(time.nanoseconds()),9,QChar('0')));
}
