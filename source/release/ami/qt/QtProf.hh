#ifndef AmiQt_QtProf_hh
#define AmiQt_QtProf_hh

#include "ami/qt/QtBase.hh"

#include "qwt_plot_curve.h"
class QwtPlot;
class QColor;

namespace Ami {
  class EntryProf;
  class AbsTransform;
  namespace Qt {
    class QtProf : public QtBase {
    public:
      QtProf(const QString&   title,
	     const Ami::EntryProf&,
	     const AbsTransform& x,
	     const AbsTransform& y,
	     const QColor&);
      ~QtProf();
    public:
      void        dump  (FILE*   ) const;
      void        attach(QwtPlot*);
      void        update()        ;
      void        xscale_update() ;
      void        yscale_update() ;
      const AxisInfo* xinfo() const;
      double      normalization() const;
    private:
      const AbsTransform&     _xscale;
      const AbsTransform&     _yscale;
      QwtPlotCurve     _curve;
      double*          _x;
      double*          _y;
      double*          _xa;
      AxisInfo*     _xinfo;
    };
  };
};

#endif
