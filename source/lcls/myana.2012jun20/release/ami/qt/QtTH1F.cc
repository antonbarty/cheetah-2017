#include "QtTH1F.hh"
#include "ami/qt/AxisArray.hh"

#include "ami/data/AbsTransform.hh"
#include "ami/data/EntryTH1F.hh"

#include "qwt_plot.h"

using namespace Ami::Qt;

QtTH1F::QtTH1F(const QString&   title,
	       const Ami::EntryTH1F& entry,
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
    _y[b] = entry.content(b);
    b++;
  }
  _x[b] = _xscale(x0 + double(nb)*dx);
  _y[b] = _y[b-1];
  _curve.setRawData(_x,_y,nb+1);  // QwtPlotCurve wants the x-endpoint
  _xinfo = new AxisArray(_x,nb);

  printf("Created TH1F %p with %d bins from signature %d\n",this,nb,entry.desc().signature());
}
  
  
QtTH1F::~QtTH1F()
{
  _curve.attach(NULL);
  delete _xinfo;
  delete[] _x;
  delete[] _y;
}

void           QtTH1F::dump  (FILE* f) const
{
  const EntryTH1F& _entry = static_cast<const EntryTH1F&>(entry());
  for(unsigned b=0; b<= _entry.desc().nbins(); b++) {
    double x = _xscale(0.5*(_x[b]+_x[b+1]));
    double y = _yscale(_y[b]);
    fprintf(f,"%g %g\n",x,y);
  }
}

void           QtTH1F::attach(QwtPlot* p)
{
  _curve.attach(p);
  if (p) {
    const EntryTH1F& _entry = static_cast<const EntryTH1F&>(entry());
    p->setAxisTitle(QwtPlot::xBottom,_entry.desc().xtitle());
    p->setAxisTitle(QwtPlot::yLeft  ,_entry.desc().ytitle());
  }
}

void           QtTH1F::update()
{
  yscale_update();
}

void QtTH1F::xscale_update()
{
  const EntryTH1F& _entry = static_cast<const EntryTH1F&>(entry());
  unsigned nb = _entry.desc().nbins();
  double xlow = _entry.desc().xlow();
  double xhi  = _entry.desc().xup ();
  double dx = nb ? (xhi - xlow) / double(nb) : 0;
  double x0 = xlow;
  for(unsigned b=0; b<=nb; b++)
    _x[b] = _xscale(x0 + double(b)*dx);
}

void QtTH1F::yscale_update()
{
  const EntryTH1F& _entry = static_cast<const EntryTH1F&>(entry());
  double scal = 1;
  if (entry().desc().isnormalized()) {
    double   n  = _entry.info(EntryTH1F::Normalization);
    scal = (n < 1) ? 1. : 1./n;
  }
  unsigned nb = _entry.desc().nbins();
  for(unsigned b=0; b<nb; b++)
    _y[b] = _yscale(_entry.content(b)*scal);
  _y[nb] = _y[nb-1];
}

const AxisInfo* QtTH1F::xinfo() const
{
  return _xinfo;
}

double QtTH1F::normalization() const
{
  return static_cast<const EntryTH1F&>(entry()).info(EntryTH1F::Normalization);
}
