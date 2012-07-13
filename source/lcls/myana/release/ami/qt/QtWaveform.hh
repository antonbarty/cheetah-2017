#ifndef AmiQt_QtWaveform_hh
#define AmiQt_QtWaveform_hh

#include "ami/qt/QtBase.hh"

#include "qwt_plot_curve.h"
class QwtPlot;
class QColor;

namespace Ami {
  class EntryWaveform;
  class AbsTransform;
  namespace Qt {
    class QtWaveform : public QtBase {
    public:
      QtWaveform(const QString&   title,
		 const EntryWaveform&, 
		 const AbsTransform& x, 
		 const AbsTransform& y,
		 const QColor& c);
      ~QtWaveform();
    public:
      void        dump  (FILE*) const;
      void        attach(QwtPlot*);
      void        update()        ;
      void        xscale_update() ;
      void        yscale_update() ;
      const AxisInfo* xinfo() const;
    private:
      const AbsTransform&   _xscale;
      const AbsTransform&   _yscale;
      QwtPlotCurve   _curve;
      double*        _x;
      double*        _y;
      AxisInfo*      _xinfo;
    };
  };
};

#endif
