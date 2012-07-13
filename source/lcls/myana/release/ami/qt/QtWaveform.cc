#include "QtWaveform.hh"
#include "ami/qt/AxisArray.hh"

#include "ami/data/AbsTransform.hh"
#include "ami/data/EntryWaveform.hh"

using namespace Ami::Qt;

QtWaveform::QtWaveform(const QString&   title,
		       const Ami::EntryWaveform& entry,
		       const AbsTransform& x,
		       const AbsTransform& y,
		       const QColor& c) :
  QtBase(title,entry),
  _xscale(x),
  _yscale(y),
  _curve(entry.desc().name())
{
  _curve.setStyle(QwtPlotCurve::Steps);
  _curve.setPen  (QPen(c));
  _curve.setCurveAttribute(QwtPlotCurve::Inverted,true);
  
  unsigned nb = entry.desc().nbins();
  double xlow = entry.desc().xlow();
  double xhi  = entry.desc().xup ();
  _x = new double[nb+1];
  _y = new double[nb+1];
  double dx = nb ? (xhi - xlow) / double(nb) : 0;
  double x0 = xlow;
  unsigned b=0;
  while(b<nb) {
    _x[b] = _xscale(x0 + double(b)*dx);
    _y[b] = _yscale(entry.content(b));
    b++;
  }
  _x[b] = _xscale(x0 + double(nb)*dx);
  _y[b] = _yscale(_y[b-1]);
  _curve.setRawData(_x,_y,nb+1);  // QwtPlotCurve wants the x-endpoint
  _xinfo = new AxisArray(_x,nb);
}
  
  
QtWaveform::~QtWaveform()
{
  _curve.attach(NULL);
  delete _xinfo;
  delete[] _x;
  delete[] _y;
}

void           QtWaveform::dump  (FILE* f) const
{
  const EntryWaveform& _entry = static_cast<const EntryWaveform&>(entry());
  for(unsigned b=0; b< _entry.desc().nbins(); b++) {
    double x = _xscale(0.5*(_x[b]+_x[b+1]));
    double y = _yscale(_y[b]);
    fprintf(f,"%g %g\n",x,y);
  }
}

void           QtWaveform::attach(QwtPlot* p)
{
  _curve.attach(p);
}

void           QtWaveform::update()
{
  yscale_update();
}

void QtWaveform::xscale_update()
{
  const EntryWaveform& _entry = static_cast<const EntryWaveform&>(entry());
  unsigned nb = _entry.desc().nbins();
  double xlow = _entry.desc().xlow();
  double xhi  = _entry.desc().xup ();
  double dx = nb ? (xhi - xlow) / double(nb) : 0;
  double x0 = xlow;
  for(unsigned b=0; b<=nb; b++)
    _x[b] = _xscale(x0 + double(b)*dx);
}

void QtWaveform::yscale_update()
{
  const EntryWaveform& _entry = static_cast<const EntryWaveform&>(entry());
  double scal = 1;
  if (entry().desc().isnormalized()) {
    double   n  = _entry.info(EntryWaveform::Normalization);
    scal = (n < 1) ? 1. : 1./n;
  }
  unsigned nb = _entry.desc().nbins();
  for(unsigned b=0; b<nb; b++)
    _y[b] = _yscale(_entry.content(b)*scal);
  _y[nb] = _y[nb-1];
}

const AxisInfo* QtWaveform::xinfo() const
{
  return _xinfo;
}
