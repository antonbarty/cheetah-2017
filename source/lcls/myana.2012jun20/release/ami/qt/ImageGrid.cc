#include "ami/qt/ImageGrid.hh"

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QFrame>

#include <math.h>

using namespace Ami::Qt;

static const unsigned grid_width = 35;

ImageGrid::ImageGrid( Axis     a, 
                      Origin   o,
                      double   y0,
                      double   dy,
                      unsigned ny ) :
  _axis  (a),
  _origin(o),
  _y0    (y0),
  _dy    (dy),
  _ny    (ny)
{
  setAlignment(::Qt::AlignLeft | ::Qt::AlignTop);
  _fill();
}

ImageGrid::~ImageGrid()
{
}

void ImageGrid::_fill()
{
  QImage* image = new QImage(grid_width, _ny, QImage::Format_ARGB32);
  
  const double major_width = 2;

  double major_step, fstep;
  if (_origin == Center) {
    double len = 0.5*_dy;
    major_step = pow(10.,floor(log10(len))-1.);
    const unsigned max_steps = 6;
    fstep = len/(major_step*double(max_steps));
  }
  else {
    double len = _dy;
    major_step = pow(10.,floor(log10(len))-1.);
    const unsigned max_steps = 13;
    fstep = len/(major_step*double(max_steps));
  }
  
  //  we have step size to within a factor of 10
  if      (fstep > 5)
    major_step *= 10;
  else if (fstep > 2)
    major_step *= 5;
  else
    major_step *= 2;

  QRgb bg = QPalette().color(QPalette::Window).rgb();
  image->fill(bg);

  QRgb fg = qRgb(0,0,0);

  QRgb* b = reinterpret_cast<QRgb*>(image->bits());
  double  y = _y0;
  double dy = _dy/double(_ny);
  for(unsigned i=0; i<_ny; i++, b+=grid_width) {
    b[0] = fg;
    b[1] = fg;

    QRgb v;
    double prox = major_width - fabs(drem(y,major_step))/dy;
    if (prox < 0) v = bg;
    else if (prox < 1) {
      int g = int(double(qGray(bg))*(1-prox) + double(qGray(fg))*prox);
      v = qRgb(g,g,g);
    }
    else v = fg;

    b[2] = v;
    b[3] = v;
    b[4] = v;

    y += dy;
  }
  
  //  setMinimumSize(image->size());
  QTransform transform(QTransform().rotate(_axis==X ? 180 : 0, ::Qt::XAxis));
  QPixmap pixmap(QPixmap::fromImage(*image).transformed(transform));
  QPainter painter(&pixmap);
  painter.setLayoutDirection(::Qt::LeftToRight);
  QFont f(painter.font());
  f.setPointSize(8);
  painter.setFont(f);

  if (_axis==X)
    y = _y0 + _dy;
  else
    y = _y0;

  double pprox = -1;
  for(unsigned i=0; i<_ny; i++, b+=grid_width) {
    double prox = major_width - fabs(drem(y,major_step))/dy;
    if (prox>=0 && pprox<0) {
      QRectF r(8, i-5, grid_width-8, 10);
      double v = floor(y/major_step+0.5)*major_step;
      painter.drawText(r, ::Qt::AlignLeft|::Qt::AlignVCenter, QString::number(v));
    }
    pprox = prox;
    if (_axis==X)
      y -= dy;
    else
      y += dy;
  }
  
  if (_axis==X)
    setPixmap(pixmap.transformed(QTransform().rotate(90)));
  else
    setPixmap(pixmap);

  delete image;
}

void ImageGrid::set_scale(double y0, double dy)
{
  _y0 = y0;
  _dy = dy;
  _fill();
}

void ImageGrid::resize(unsigned ny)
{
  _ny = ny;
  _fill();
}
