#include "QtProf.hh"
#include "ami/qt/AxisArray.hh"

#include "ami/data/AbsTransform.hh"
#include "ami/data/EntryProf.hh"

#include "qwt_plot.h"
#include "qwt_symbol.h"

using namespace Ami::Qt;

QtProf::QtProf(const QString&   title,
	       const Ami::EntryProf& entry,
	       const AbsTransform& x,
	       const AbsTransform& y,
	       const QColor& c) :
  QtBase(title,entry),
  _xscale(x),
  _yscale(y),
  _curve(entry.desc().name())
{
  _curve.setStyle(QwtPlotCurve::Dots);
  QwtSymbol symbol(QwtSymbol::Diamond,QBrush(c),QPen(c),QSize(5,5));
  _curve.setSymbol(symbol);
  _curve.setPen  (QPen(c));
  _curve.setCurveAttribute(QwtPlotCurve::Inverted,true);
  
  unsigned nb = entry.desc().nbins();
  double xlow = entry.desc().xlow();
  double xhi  = entry.desc().xup ();
  _xa = new double[nb];
  _x  = new double[nb];
  _y  = new double[nb];
  double dx = nb ? (xhi - xlow) / double(nb) : 0;
  double x0 = xlow;
  unsigned b=0;
  unsigned i=0;
  while(b<nb) {
    _xa[b] = _xscale(x0 + double(b)*dx);
    if (entry.nentries(b)) {
      _x[i] = _xscale(x0 + double(b)*dx);
      _y[i] = entry.ymean(b);
      i++;
    }
    b++;
  }
  _curve.setRawData(_x,_y,i);  // QwtPlotCurve wants the x-endpoint
  _xinfo = new AxisArray(_xa,nb);
}
  
  
QtProf::~QtProf()
{
  _curve.attach(NULL);
  delete _xinfo;
  delete[] _xa;
  delete[] _x;
  delete[] _y;
}

void           QtProf::dump  (FILE* f) const
{
  for(unsigned b=0; b<_curve.data().size(); b++) {
    double x = _x[b];
    double y = _y[b];
    fprintf(f,"%g %g\n",x,y);
  }
}

void           QtProf::attach(QwtPlot* p)
{
  _curve.attach(p);
  if (p) {
    const EntryProf& _entry = static_cast<const EntryProf&>(entry());
    p->setAxisTitle(QwtPlot::xBottom,_entry.desc().xtitle());
    p->setAxisTitle(QwtPlot::yLeft  ,_entry.desc().ytitle());
  }
}

void           QtProf::update()
{
  const EntryProf& _entry = static_cast<const EntryProf&>(entry());
  unsigned nb = _entry.desc().nbins();
  double xlow = _entry.desc().xlow();
  double xhi  = _entry.desc().xup ();
  double dx = nb ? (xhi - xlow) / double(nb) : 0;
  double x0 = xlow;
  unsigned i=0;
  for(unsigned b=0; b<nb; b++)
    if (_entry.nentries(b)) {
      _x[i] = _xscale(x0 + double(b)*dx);
      _y[i] = _entry.ymean(b);
      i++;
    }
  _curve.setRawData(_x,_y,i);  // QwtPlotCurve wants the x-endpoint
}

void QtProf::xscale_update()
{
  update();
}

void QtProf::yscale_update()
{
  update();
}

const AxisInfo* QtProf::xinfo() const
{
  return _xinfo;
}

double QtProf::normalization() const
{
  return static_cast<const EntryProf&>(entry()).info(EntryProf::Normalization);
}
