#include "AnnulusCursors.hh"

#include "ami/qt/AxisInfo.hh"
#include "ami/qt/ImageFrame.hh"
#include "ami/qt/QtPersistent.hh"

#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

using namespace Ami::Qt;

static void draw_line(double _xc, double _yc,
		      double _r0, double _r1,
		      double _f , QImage& image);
static void draw_arc (double _xc, double _yc,
		      double _f0, double _f1,
		      double _r , QImage& image);


static const QChar RHO(0x03c1);
static const QChar PHI(0x03c6);
static const QChar DEG(0x00b0);
static const double DEG_TO_RAD = M_PI/180.;
static const double RAD_TO_DEG = 180./M_PI;

AnnulusCursors::AnnulusCursors(ImageFrame& f) :
  QWidget(0),
  _frame(f),
  _xc(f.size().width()/2),
  _yc(f.size().height()/2),
  _r0(f.size().width()/4),
  _r1(f.size().width()/2),
//   _f0(-M_PI),
//   _f1( M_PI),
  _f0(0),
  _f1(0),
  _edit_xc   (new QLineEdit),
  _edit_yc   (new QLineEdit),
  _edit_inner(new QLineEdit),
  _edit_outer(new QLineEdit),
  _edit_phi0 (new QLineEdit),
  _edit_phi1 (new QLineEdit)
{
  _edit_xc   ->setMaximumWidth(40);
  _edit_yc   ->setMaximumWidth(40);
  _edit_inner->setMaximumWidth(40);
  _edit_outer->setMaximumWidth(40);
  _edit_phi0 ->setMaximumWidth(40);
  _edit_phi1 ->setMaximumWidth(40);

  new QDoubleValidator(_edit_xc);
  new QDoubleValidator(_edit_yc);
  new QDoubleValidator(_edit_inner);
  new QDoubleValidator(_edit_outer);
  new QDoubleValidator(_edit_phi0);
  new QDoubleValidator(_edit_phi1);

  QPushButton* grab_center = new QPushButton("Grab");
  QPushButton* grab_limits = new QPushButton("Grab");

  QGridLayout* layout = new QGridLayout;
  layout->addWidget(new QLabel("center"),0,0);
  layout->addWidget(_edit_xc            ,0,1);
  layout->addWidget(_edit_yc            ,0,2);
  layout->addWidget(grab_center         ,0,3);

  layout->addWidget(new QLabel(QString("r inner,%1 begin [%2]").arg(PHI).arg(DEG)) ,1,0);
  layout->addWidget(_edit_inner         ,1,1);
  layout->addWidget(_edit_phi1          ,1,2);

  layout->addWidget(new QLabel(QString("r outer,%1 end [%2]").arg(PHI).arg(DEG)) ,2,0);
  layout->addWidget(_edit_outer         ,2,1);
  layout->addWidget(_edit_phi0          ,2,2);
  layout->addWidget(grab_limits         ,1,3,2,1);

  setLayout(layout);

  connect(grab_center, SIGNAL(clicked()), this, SLOT(grab_center()));
  connect(grab_limits, SIGNAL(clicked()), this, SLOT(grab_limits()));

  connect(_edit_xc   , SIGNAL(editingFinished()), this, SLOT(update_edits()));
  connect(_edit_yc   , SIGNAL(editingFinished()), this, SLOT(update_edits()));
  connect(_edit_inner, SIGNAL(editingFinished()), this, SLOT(update_edits()));
  connect(_edit_outer, SIGNAL(editingFinished()), this, SLOT(update_edits()));
  connect(_edit_phi0 , SIGNAL(editingFinished()), this, SLOT(update_edits()));
  connect(_edit_phi1 , SIGNAL(editingFinished()), this, SLOT(update_edits()));

  _set_edits();
}

AnnulusCursors::~AnnulusCursors()
{
}

void AnnulusCursors::save(char*& p) const
{
  XML_insert(p, "double", "_xc", QtPersistent::insert(p,_xc) );
  XML_insert(p, "double", "_yc", QtPersistent::insert(p,_yc) );
  XML_insert(p, "double", "_r0", QtPersistent::insert(p,_r0) );
  XML_insert(p, "double", "_r1", QtPersistent::insert(p,_r1) );
  XML_insert(p, "double", "_f0", QtPersistent::insert(p,_f0) );
  XML_insert(p, "double", "_f1", QtPersistent::insert(p,_f1) );
}

void AnnulusCursors::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if      (tag.name == "_xc")
      _xc = QtPersistent::extract_i(p);
    else if (tag.name == "_yc")
      _yc = QtPersistent::extract_i(p);
    else if (tag.name == "_r0")
      _r0 = QtPersistent::extract_i(p);
    else if (tag.name == "_r1")
      _r1 = QtPersistent::extract_i(p);
    else if (tag.name == "_f0")
      _f0 = QtPersistent::extract_i(p);
    else if (tag.name == "_f1")
      _f1 = QtPersistent::extract_i(p);
  XML_iterate_close(AnnulusCursors,tag);
  _set_edits();
}

void AnnulusCursors::grab_center() { _active = Center; _frame.set_cursor_input(this); }
void AnnulusCursors::grab_limits() { _active = Limits; _frame.set_cursor_input(this); }

void AnnulusCursors::update_edits() 
{
  _xc = _edit_xc   ->text().toDouble();
  _yc = _edit_yc   ->text().toDouble();
  _r0 = _edit_inner->text().toDouble();
  _r1 = _edit_outer->text().toDouble();
  _f0 = _edit_phi0 ->text().toDouble()*DEG_TO_RAD;
  _f1 = _edit_phi1 ->text().toDouble()*DEG_TO_RAD;
  emit changed();
}

void AnnulusCursors::_set_edits()
{
  _edit_xc   ->setText(QString::number(_xc));
  _edit_yc   ->setText(QString::number(_yc));
  _edit_inner->setText(QString::number(_r0));
  _edit_outer->setText(QString::number(_r1));
  _edit_phi0 ->setText(QString::number(_f0*RAD_TO_DEG));
  _edit_phi1 ->setText(QString::number(_f1*RAD_TO_DEG));
  _edit_xc   ->setCursorPosition(0);
  _edit_yc   ->setCursorPosition(0);
  _edit_inner->setCursorPosition(0);
  _edit_outer->setCursorPosition(0);
  _edit_phi0 ->setCursorPosition(0);
  _edit_phi1 ->setCursorPosition(0);
}

void AnnulusCursors::draw(QImage& image)
{
  const unsigned char c = 0xff;
  const QSize& sz = image.size();

  const AxisInfo& xinfo = *_frame.xinfo();
  const AxisInfo& yinfo = *_frame.yinfo();

  int xc = xinfo.tick_u(_xc);
  int yc = yinfo.tick_u(_yc);

  if (xc >= 5 && (xc+5)<sz.width() &&
      yc >= 5 && (yc+5)<sz.height()) {
    // draw center cross
    for(int i=-5; i<=5; i++) {
      if ((i+xc)>=0 && (i+xc)<sz.width())      *(image.scanLine(yc+0)+xc+i) = c;
      if ((i+yc)>=0 && (i+yc)<sz.height())     *(image.scanLine(yc+i)+xc+0) = c;
    }
  }

  //  Assuming x scale factor is same as y scale factor
  double scale = double(xinfo.hi()-xinfo.lo())/(xinfo.position(xinfo.hi())-xinfo.position(xinfo.lo()));
  double r0 = _r0*scale;
  double r1 = _r1*scale;
  if (_f0 != _f1) {
    draw_line(xc,yc,r0,r1,_f0,image);    // draw clockwise angular boundary
    draw_line(xc,yc,r0,r1,_f1,image);    // draw counterclockwise angular boundary
  }
  double f1 = _f0 < _f1 ? _f1 : _f1 + 2*M_PI;
  draw_arc(xc,yc,_f0,f1,r0,image);  // draw inner arc
  draw_arc(xc,yc,_f0,f1,r1,image);  // draw outer arc
}

void AnnulusCursors::mousePressEvent(double x,double y)
{
  switch(_active) {
  case Center:  _xc=x; _yc=y; break;
  case Limits:  
    _r0=sqrt(pow(x-_xc,2)+pow(y-_yc,2)); 
    _f1=atan2(y-_yc,x-_xc); 
    break;
  default: break;
  }
}

void AnnulusCursors::mouseMoveEvent   (double x,double y)
{
  if (_active==Limits) {
    _r1=sqrt(pow(x-_xc,2)+pow(y-_yc,2));
    _f0=atan2(y-_yc,x-_xc); 
    _set_edits();
    emit changed();
  }
}

void AnnulusCursors::mouseReleaseEvent(double x,double y) 
{
  _frame.set_cursor_input(0);
  mouseMoveEvent(x,y);
  if (_active==Limits) {
    if (_r1<_r0) {
      double r0 = _r0;
      _r0 = _r1;
      _r1 = r0;
    }
  }
  _active=None;
  _set_edits();
  emit changed();
}

static double clip_r(double r, double a, double x0, double x1)
{
  if (r*a < x0 ) r = x0/a;
  if (r*a > x1)  r = x1/a;
  return r;
}

void draw_line(double _xc, double _yc,
	       double _r0, double _r1,
	       double _f, QImage& image)
{
  const QSize& sz = image.size();
  double cosf = cos(_f);
  double tanf = tan(_f);
  double sinf = tanf*cosf;
  double cotf = 1./tanf;

  //  clip to image size
  double xmax = double(sz.width() -1) - _xc;
  double ymax = double(sz.height()-1) - _yc;
  double r0;
  r0 = clip_r (_r0, cosf, -_xc, xmax);
  r0 = clip_r ( r0, sinf, -_yc, ymax);
  double r1;
  r1 = clip_r (_r1, cosf, -_xc, xmax);
  r1 = clip_r ( r1, sinf, -_yc, ymax);
  
  //  draw
  double dx = (r1-r0)*cosf;
  double dy = (r1-r0)*sinf;
  if (fabs(dx) > fabs(dy)) {
    if (dx > 0)
      for(double x=r0*cosf; x<=r1*cosf; x++) {
	double y=x*tanf;
	*(image.scanLine(unsigned(y+_yc))+unsigned(x+_xc)) = 0xff;
      }
    else
      for(double x=r1*cosf; x<=r0*cosf; x++) {
	double y=x*tanf;
	*(image.scanLine(unsigned(y+_yc))+unsigned(x+_xc)) = 0xff;
      }
  }
  else {
    if (dy > 0)
      for(double y=r0*sinf; y<=r1*sinf; y++) {
	double x=y*cotf;
	*(image.scanLine(unsigned(y+_yc))+unsigned(x+_xc)) = 0xff;
      }
    else
      for(double y=r1*sinf; y<=r0*sinf; y++) {
	double x=y*cotf;
	*(image.scanLine(unsigned(y+_yc))+unsigned(x+_xc)) = 0xff;
      }
  }
}


void draw_arc (double _xc, double _yc,
	       double _f0, double _f1,
	       double _r, QImage& image)
{
  if (_r!=0) {
    const QSize& sz = image.size();
    double df = 1./_r;
    for(double f=_f0; f<=_f1; f+=df) {
      int ix = int(_r*cos(f)+_xc);
      int iy = int(_r*sin(f)+_yc);
      if (ix>=0 && ix < sz.width() &&
	  iy>=0 && iy < sz.height())
	*(image.scanLine(iy)+ix) = 0xff;
    }
  }
}

unsigned AnnulusCursors::nrbins() const
{
  const AxisInfo& info = *_frame.xinfo();
  return info.tick(r_outer())-info.tick(r_inner());
}
