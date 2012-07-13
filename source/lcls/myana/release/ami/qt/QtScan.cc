#include "QtScan.hh"
#include "ami/qt/AxisArray.hh"

#include "ami/data/AbsTransform.hh"
#include "ami/data/EntryScan.hh"

#include "qwt_plot.h"
#include "qwt_symbol.h"

using namespace Ami::Qt;

QtScan::QtScan(const QString&   title,
	       const Ami::EntryScan& entry,
	       const AbsTransform& x,
	       const AbsTransform& y,
	       const QColor& c) :
  QtBase(title,entry),
  _xscale(x),
  _yscale(y),
  _curve(entry.desc().name())
{
  if (entry.desc().scatter()) {
    _curve.setStyle(QwtPlotCurve::Dots);
    QwtSymbol symbol(QwtSymbol::Diamond,QBrush(c),QPen(c),QSize(5,5));
    _curve.setSymbol(symbol);
  }
  else {
    _curve.setStyle(QwtPlotCurve::Lines);
  }
  _curve.setPen  (QPen(c));
  //  _curve.setCurveAttribute(QwtPlotCurve::Inverted,true);
  
  unsigned nb = entry.desc().nbins();
  _xa = new double[nb];
  _x  = new double[nb];
  _y  = new double[nb];
  unsigned b=0;
  unsigned i=0;
  while(b<nb) {
    _xa[b] = _xscale(entry.xbin(b));
    if (entry.nentries(b)) {
      _x[i] = _xscale(entry.xbin(b));
      _y[i] = entry.ymean(b);
      i++;
    }
    b++;
  }
  _curve.setRawData(_x,_y,i);  // QwtPlotCurve wants the x-endpoint
  _xinfo = new AxisArray(_xa,nb);
}
  
  
QtScan::~QtScan()
{
  _curve.attach(NULL);
  delete _xinfo;
  delete[] _xa;
  delete[] _x;
  delete[] _y;
}

void           QtScan::dump  (FILE* f) const
{
  for(unsigned b=0; b<_curve.data().size(); b++) {
    double x = _x[b];
    double y = _y[b];
    fprintf(f,"%g %g\n",x,y);
  }
}

void           QtScan::attach(QwtPlot* p)
{
  _curve.attach(p);
  if (p) {
    const EntryScan& _entry = static_cast<const EntryScan&>(entry());
    p->setAxisTitle(QwtPlot::xBottom,_entry.desc().xtitle());
    p->setAxisTitle(QwtPlot::yLeft  ,_entry.desc().ytitle());
  }
}

void           QtScan::update()
{
  const EntryScan& _entry = static_cast<const EntryScan&>(entry());
  unsigned nb = _entry.desc().nbins();
  unsigned i=0;
  for(unsigned b=0; b<nb; b++)
    if (_entry.nentries(b)) {
      _x[i] = _xscale(_entry.xbin(b));
      _y[i] = _entry.ymean(b);
      i++;
    }
  _curve.setRawData(_x,_y,i);  // QwtPlotCurve wants the x-endpoint
}

void QtScan::xscale_update()
{
  update();
}

void QtScan::yscale_update()
{
  update();
}

const AxisInfo* QtScan::xinfo() const
{
  return _xinfo;
}
