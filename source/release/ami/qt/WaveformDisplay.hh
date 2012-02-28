#ifndef AmiQt_WaveformDisplay_hh
#define AmiQt_WaveformDisplay_hh

#include "ami/qt/Display.hh"
#include <QtGui/QWidget>

#include "ami/service/Semaphore.hh"
#include <list>

class QwtPlotGrid;

namespace Ami {
  class DescEntry;
  namespace Qt {
    class AxisInfo;
    class AxisBins;
    class AxisControl;
    class Transform;
    class PlotFrame;
    class WaveformDisplay : public QWidget,
			    public Display {
      Q_OBJECT
    public:
      WaveformDisplay();
      ~WaveformDisplay();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      void prototype(const Ami::DescEntry*);
      void add   (QtBase*, bool);
      void reset ();
      void show  (QtBase*);
      void hide  (QtBase*);
      const AbsTransform& xtransform() const;
      void update();
      bool canOverlay() const { return true; }
      QWidget* widget() { return this; }
    public:
      void save_plots(const QString&) const;
    public:
      const std::list<QtBase*> plots() const;
      const AxisInfo&     xinfo     () const;
      PlotFrame*          plot      () const;
    public slots:
      void save_image();
      void save_data();
      void save_reference();
      void xtransform_update();
      void xrange_change();
      void yrange_change();
      void set_plot_title();
      void set_xaxis_title();
      void set_yaxis_title();
      void toggle_grid();
      void toggle_minor_grid();
    signals:
      void redraw();
    private:
      PlotFrame*   _plot;
      QwtPlotGrid* _grid;

      Transform*   _xtransform;
      AxisControl* _xrange;
      AxisControl* _yrange;
      AxisBins* _xbins;
      const AxisInfo* _xinfo;
      const AxisInfo* _yinfo;
      std::list<QtBase*>  _curves;
      std::list<QtBase*>  _hidden;
      mutable Ami::Semaphore _sem;
    };
  };
};


#endif
