#include "QtChart.hh"
#include "ami/qt/AxisArray.hh"

#include "ami/data/EntryScalar.hh"
#include "pdsdata/xtc/ClockTime.hh"

#include <QtCore/QTime>

#include "qwt_plot.h"
#include "qwt_scale_draw.h"

namespace Ami {
  namespace Qt {
    class TimeScale : public QwtScaleDraw {
    public:
      TimeScale() {}
    
      virtual QwtText label(double v) const
      {
	time_t t = time_t(v);
	struct tm* tm_ptr = localtime(&t);
	QTime tim(tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
	tim.addMSecs(int(1000*(v-t)));
	return tim.toString();
      }
    };
  };
};

using namespace Ami::Qt;

QtChart::QtChart(const QString&   title,
		 const Ami::EntryScalar& entry,
		 const QColor& c) :
  QtBase  (title,entry),
  _cache  (*new EntryScalar(entry.desc())),
  _n      (entry.desc().npoints()),
  _skip   (0),
  _current(0),
  _curve  (entry.desc().name()),
  _pts    (0)
{
  _curve.setStyle(QwtPlotCurve::Steps);
  _curve.setPen  (QPen(c));
  _curve.setCurveAttribute(QwtPlotCurve::Inverted,true);

  unsigned nb = 2*_n;
  _x = new double[nb];
  _y = new double[nb];

  const Pds::ClockTime& tv = entry.time();
  double time = double(tv.seconds()) + 1.e-9*double(tv.nanoseconds());
  for(unsigned k=0; k<nb; k++) {
    _x[k] = time;
    _y[k] = 0;
  }

  _curve.setRawData(_x,_y,0);  // QwtPlotCurve wants the x-endpoint
  _xinfo = new AxisArray(_x,nb);
}
  
  
QtChart::~QtChart()
{
  _curve.attach(NULL);
  delete _xinfo;
  delete[] _x;
  delete[] _y;
  delete &_cache;
}

void           QtChart::dump  (FILE* f) const
{
  for(unsigned b=_current; b<_current+_n; b++) {
    fprintf(f,"%.2f %g\n",_x[b],_y[b]);
  }
}

void           QtChart::attach(QwtPlot* p)
{
  _curve.attach(p);
  p->setAxisScaleDraw(QwtPlot::xBottom, new TimeScale);
  p->setAxisLabelRotation (QwtPlot::xBottom, -50.0);
  p->setAxisLabelAlignment(QwtPlot::xBottom, ::Qt::AlignLeft | ::Qt::AlignBottom);
  p->setAxisTitle(QwtPlot::xBottom,"Time [sec]");

  p->setAxisTitle(QwtPlot::yLeft,entry().desc().ytitle());
}

void           QtChart::update()
{
  if (_skip--)
    return;

  const EntryScalar& entry = static_cast<const EntryScalar&>(QtBase::entry());
  _skip = entry.desc().prescale()-1;

  if (!entry.valid())
    return;

  double n = entry.entries() - _cache.entries();
  if (n>0) {
    const Pds::ClockTime& tv = entry.time();
    double time = double(tv.seconds()) + 1.e-9*double(tv.nanoseconds());

    if (_cache.entries()==0) {
      for(unsigned k=0; k<2*_n; k++) {
	_x[k] = time;
	_y[k] = 0;
      }
    }

    _x[_current+0 ] = time;
    _x[_current+_n] = time;
    
    // calculate y
    double y;
    if (n>0) {
      switch(entry.desc().stat()) {
      case Ami::DescScalar::StdDev:
        y = (entry.sum() - _cache.sum());
        y = (entry.sqsum() - _cache.sqsum() - y*y/n)/n;
        y = (y>0) ? sqrt(y) : 0;
        break;
      case Ami::DescScalar::Mean:
      default:
        y = (entry.sum() - _cache.sum())/n;
        break;
      }
    }
    else
      y = 0;

    _cache.setto(entry);
    _y[_current]    = y;
    _y[_current+_n] = y;
    
    if (++_current >= _n)
      _current = 0;

    if (_pts++ < _n) 
      _curve.setRawData(&_x[0],&_y[0], _pts);
    else
      _curve.setRawData(&_x[_current],&_y[_current], _n);
  }
}

void QtChart::xscale_update()
{
}

void QtChart::yscale_update()
{
}

const AxisInfo* QtChart::xinfo() const
{
  return 0;
}
