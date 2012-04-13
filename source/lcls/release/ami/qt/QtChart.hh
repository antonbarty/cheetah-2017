 #ifndef AmiQt_QtChart_hh
#define AmiQt_QtChart_hh

#include "ami/qt/QtBase.hh"

#include "ami/data/EntryScalar.hh"

#include "qwt_plot_curve.h"
class QwtPlot;
class QColor;

namespace Ami {
  class EntryChart;
  namespace Qt {
    class QtChart : public QtBase {
    public:
      QtChart(const QString&   title,
	      const Ami::EntryScalar&,
	      const QColor&);
      virtual ~QtChart();
    public:
      void        dump  (FILE*   ) const;
      void        attach(QwtPlot*);
      void        update()        ;
      void        xscale_update() ;
      void        yscale_update() ;
      const AxisInfo* xinfo() const;
    private:
      EntryScalar&     _cache;
      unsigned         _n;  // max # of pts
      unsigned         _skip;
      unsigned         _current;
      QwtPlotCurve     _curve;
      unsigned         _pts; // accumulate # of pts
      double*          _x;
      double*          _y;
      AxisInfo*     _xinfo;
    };
  };
};

#endif
