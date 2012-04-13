#include "QtTH2F.hh"
#include "ami/qt/AxisArray.hh"

#include "ami/data/AbsTransform.hh"
#include "ami/data/EntryTH2F.hh"

#include "qwt_plot.h"
#include "qwt_plot_layout.h"
#include "qwt_raster_data.h"
#include "qwt_scale_widget.h"
#include "qwt_color_map.h"

#include <QtGui/QPen>
namespace Ami {
  namespace Qt {
    class QtTH2F::DataCache : public QwtRasterData {
    public:
      DataCache(const Ami::EntryTH2F& entry) : _entry(entry) 
      {
        const DescTH2F& desc = entry.desc();
        _idx = double(desc.nbinsx())/(desc.xup()-desc.xlow());
        _idy = double(desc.nbinsy())/(desc.yup()-desc.ylow());
        setBoundingRect(QwtDoubleRect(desc.xlow(),desc.xup(),
                                      desc.ylow(),desc.yup()));
        update();
      }
      ~DataCache() {}
    public:
      //  very slow
      double value(double x,double y) const {
        const DescTH2F& desc = _entry.desc();
        int ix = int((x-desc.xlow())*_idx);
        int iy = int((y-desc.ylow())*_idy);
        return _entry.content(ix,iy);
      }
      QwtRasterData* copy() const {
        return const_cast<DataCache*>(this);
      }
      QwtDoubleInterval range() const {
        return QwtDoubleInterval(_vlo,_vhi);
      }
      QSize rasterHint ( const QwtDoubleRect& rect) const {
        const DescTH2F& desc = _entry.desc();
        return QSize(desc.nbinsx(),desc.nbinsy());
      }
    public:
      void update() {
        _vlo = _vhi = _entry.content(0,0);
        const Ami::DescTH2F& d = _entry.desc();
        for(unsigned i=0; i<d.nbinsy(); i++)
          for(unsigned j=0; j<d.nbinsx(); j++) {
            double z = _entry.content(j,i);
            if (z < _vlo) _vlo = z;
            if (z > _vhi) _vhi = z;
          }
      }
    private:
      double _idx, _idy;
      double _vlo, _vhi;
      const EntryTH2F& _entry;
    };
  };
};

using namespace Ami::Qt;

QtTH2F::QtTH2F(const QString&   title,
	       const Ami::EntryTH2F& entry) :
  QtBase(title,entry),
  _curve(entry.desc().name()),
  _z    (new DataCache(entry))
{
  _curve.setData    (*_z);

  _curve.setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
  _curve.setDefaultContourPen(QPen(::Qt::NoPen));
  QwtLinearColorMap map = QwtLinearColorMap(::Qt::black, ::Qt::white); 
  for(unsigned k=0; k<40; k++)
    map.addColorStop(double(  0+k)/255., QColor(k*6,0,0));
  for(unsigned k=0; k<86; k++)
    map.addColorStop(double( 43+k)/255., QColor(255-k*3,k*3,0));
  for(unsigned k=0; k<86; k++)
    map.addColorStop(double(129+k)/255., QColor(0,255-k*3,k*3));
  for(unsigned k=0; k<40; k++)
    map.addColorStop(double(215+k)/255., QColor(k*3,0,255-k*3));
  map.addColorStop(1.,QColor(255,255,255));
  _curve.setColorMap(map);
}
  
  
QtTH2F::~QtTH2F()
{
  _curve.attach(NULL);
}

void           QtTH2F::dump  (FILE* f) const
{
  const DescTH2F&  _desc  = static_cast<const DescTH2F& >(entry().desc());
  
  fprintf(f,"%g %g %g %g\n",
          _desc.xlow(),_desc.xup(),
          _desc.ylow(),_desc.yup());
  double dx = (_desc.xup()-_desc.xlow())/double(_desc.nbinsx());
  double dy = (_desc.yup()-_desc.ylow())/double(_desc.nbinsy());
  double y = _desc.ylow()+0.5*dy;
  for(unsigned iy=0; iy<_desc.nbinsy(); iy++, y+=dy) {
    double x = _desc.xlow()+0.5*dx;
    for(unsigned ix=0; ix<_desc.nbinsx(); ix++, x+=dx)
      fprintf(f,"%g ",_z->value(x,y));
    fprintf(f,"\n");
  }
}

void           QtTH2F::attach(QwtPlot* p)
{
  _curve.attach(p);
  if (p) {
    const DescTH2F& _desc = static_cast<const EntryTH2F&>(entry()).desc();
    p->setAxisTitle(QwtPlot::xBottom, _desc.xtitle());
    p->setAxisScale(QwtPlot::xBottom, _desc.xlow(), _desc.xup());
    p->setAxisTitle(QwtPlot::yLeft  , _desc.ytitle());
    p->setAxisScale(QwtPlot::yLeft  , _desc.ylow(), _desc.yup());

    QwtScaleWidget* a = p->axisWidget(QwtPlot::yRight);
    a->setColorBarEnabled(true);
    a->setColorMap(_z->range(),_curve.colorMap());
    p->setAxisScale(QwtPlot::yRight, _z->range().minValue(), _z->range().maxValue());
    p->enableAxis  (QwtPlot::yRight);
    p->plotLayout()->setAlignCanvasToScales(true);  

    _colorBar = a;
    _plot = p;
  }
}

void           QtTH2F::update()
{
  _z->update();
  _colorBar->setColorMap(_z->range(),_curve.colorMap());
  _plot->setAxisScale(QwtPlot::yRight, _z->range().minValue(), _z->range().maxValue());
}

void QtTH2F::xscale_update()
{
}

void QtTH2F::yscale_update()
{
}

const AxisInfo* QtTH2F::xinfo() const
{
  return 0;
}

double QtTH2F::normalization() const
{
  return static_cast<const EntryTH2F&>(entry()).info(EntryTH2F::Normalization);
}
