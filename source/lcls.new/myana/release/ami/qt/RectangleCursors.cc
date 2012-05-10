#include "RectangleCursors.hh"

#include "ami/qt/AxisInfo.hh"
#include "ami/qt/ImageFrame.hh"
#include "ami/qt/QtPersistent.hh"

#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

using namespace Ami::Qt;

RectangleCursors::RectangleCursors(ImageFrame& f) :
  QWidget(0),
  _frame(f),
  _x0(f.size().width()/4),
  _y0(f.size().width()/4),
  _x1(f.size().width()*3/4),
  _y1(f.size().width()*3/4),
  _edit_x0   (new QLineEdit),
  _edit_y0   (new QLineEdit),
  _edit_x1   (new QLineEdit),
  _edit_y1   (new QLineEdit),
  _xmax      (-1),
  _ymax      (-1),
  _delta_x   (new QLabel),
  _delta_y   (new QLabel)
{
  _edit_x0->setMaximumWidth(40);
  _edit_y0->setMaximumWidth(40);
  _edit_x1->setMaximumWidth(40);
  _edit_y1->setMaximumWidth(40);

  new QDoubleValidator(_edit_x0);
  new QDoubleValidator(_edit_y0);
  new QDoubleValidator(_edit_x1);
  new QDoubleValidator(_edit_y1);

  QPushButton* grab = new QPushButton("Grab");

  QGridLayout* layout = new QGridLayout;
  int row = 0;
  layout->addWidget(new QLabel("column"),  row, 1, ::Qt::AlignHCenter);
  layout->addWidget(new QLabel("row")   ,  row, 2, ::Qt::AlignHCenter);
  row++;
  layout->addWidget(new QLabel("top-left"),row,0);
  layout->addWidget(_edit_x0              ,row,1);
  layout->addWidget(_edit_y0              ,row,2);
  row++;
  layout->addWidget(new QLabel("btm-rght"),row,0);
  layout->addWidget(_edit_x1              ,row,1);
  layout->addWidget(_edit_y1              ,row,2);
  
  layout->addWidget(grab                ,1,3,2,1);

  row++;
  layout->addWidget(new QLabel(QString(QChar(0x0394))),row,0);
  layout->addWidget(_delta_x              ,row,1);
  layout->addWidget(_delta_y              ,row,2);
  
  setLayout(layout);

  connect(grab, SIGNAL(clicked()), this, SLOT(grab()));

  connect(_edit_x0   , SIGNAL(editingFinished()), this, SLOT(update_edits()));
  connect(_edit_y0   , SIGNAL(editingFinished()), this, SLOT(update_edits()));
  connect(_edit_x1   , SIGNAL(editingFinished()), this, SLOT(update_edits()));
  connect(_edit_y1   , SIGNAL(editingFinished()), this, SLOT(update_edits()));

  _set_edits();
}

RectangleCursors::~RectangleCursors()
{
}

void RectangleCursors::save(char*& p) const
{
  XML_insert(p, "int", "_x0", QtPersistent::insert(p,_x0) );
  XML_insert(p, "int", "_y0", QtPersistent::insert(p,_y0) );
  XML_insert(p, "int", "_x1", QtPersistent::insert(p,_x1) );
  XML_insert(p, "int", "_y1", QtPersistent::insert(p,_y1) );
}

void RectangleCursors::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "_x0")
      _x0 = QtPersistent::extract_i(p);
    else if (tag.name == "_y0")
      _y0 = QtPersistent::extract_i(p);
    else if (tag.name == "_x1")
      _x1 = QtPersistent::extract_i(p);
    else if (tag.name == "_y1")
      _y1 = QtPersistent::extract_i(p);
  XML_iterate_close(RectangleCursors,tag);

  _set_edits();
}

void RectangleCursors::grab() { _frame.set_cursor_input(this); }
void RectangleCursors::update_edits() 
{
  _x0 = _edit_x0   ->text().toDouble();
  _y0 = _edit_y0   ->text().toDouble();
  _x1 = _edit_x1   ->text().toDouble();
  _y1 = _edit_y1   ->text().toDouble();

  if (_x0 > _xmax) _x0 = _xmax;
  if (_y0 > _ymax) _y0 = _ymax;
  if (_x1 > _xmax) _x1 = _xmax;
  if (_y1 > _ymax) _y1 = _ymax;

  _set_edits();

  emit changed();
}

void RectangleCursors::_set_edits()
{
  _edit_x0   ->setText(QString::number(_x0));
  _edit_y0   ->setText(QString::number(_y0));
  _edit_x1   ->setText(QString::number(_x1));
  _edit_y1   ->setText(QString::number(_y1));
  _delta_x   ->setText(QString::number(_x1-_x0));
  _delta_y   ->setText(QString::number(_y1-_y0));
}

void RectangleCursors::draw(QImage& image)
{
  const unsigned char c = 0xff;
  const QSize& sz = image.size();

  const AxisInfo& xinfo = *_frame.xinfo();
  const AxisInfo& yinfo = *_frame.yinfo();
  _xmax = (unsigned) (xinfo.position(sz.width())-1);
  _ymax = (unsigned) (yinfo.position(sz.height())-1);

  unsigned jlo = unsigned(xinfo.tick(xlo())), jhi = unsigned(xinfo.tick(xhi()));
  unsigned klo = unsigned(yinfo.tick(ylo())), khi = unsigned(yinfo.tick(yhi()));

//   if (jhi > _xmax) jhi=_xmax;
//   if (khi > _ymax) khi=_ymax;

  { unsigned char* cc0 = image.scanLine(klo) + jlo;
    unsigned char* cc1 = image.scanLine(khi) + jlo;
    for(unsigned j=jlo; j<=jhi; j++) {
      *cc0++ = c;
      *cc1++ = c;
    }
  }

  { for(unsigned k=klo; k<khi; k++) {
      *(image.scanLine(k+1)+jlo) = c;
      *(image.scanLine(k+1)+jhi) = c;
    }
  }
}

void RectangleCursors::mousePressEvent(double x,double y)
{
  _x0=x; _y0=y;

  if (_x0 > _xmax) _x0 = _xmax;
  if (_y0 > _ymax) _y0 = _ymax;
}

void RectangleCursors::mouseMoveEvent (double x,double y)
{
  _x1=x; _y1=y;

  if (_x1 > _xmax) _x1 = _xmax;
  if (_y1 > _ymax) _y1 = _ymax;

  _set_edits();
  emit changed();
}

void RectangleCursors::mouseReleaseEvent(double x,double y) 
{
  _frame.set_cursor_input(0);
  mouseMoveEvent(x,y);
}

double RectangleCursors::xlo() const { return _x0 < _x1 ? _x0 : _x1; }
double RectangleCursors::ylo() const { return _y0 < _y1 ? _y0 : _y1; }
double RectangleCursors::xhi() const { return _x0 < _x1 ? _x1 : _x0; }
double RectangleCursors::yhi() const { return _y0 < _y1 ? _y1 : _y0; }

