#ifndef AmiQt_QtPlot_hh
#define AmiQt_QtPlot_hh

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>

class QLabel;
class QwtPlot;
class QwtPlotGrid;

namespace Ami {
  namespace Qt {
    class AxisControl;
    class QtPlot : public QtPWidget {
      Q_OBJECT
    public:
      explicit QtPlot(QWidget*       parent,
                      const QString& name);
      QtPlot(QWidget*       parent);
      virtual ~QtPlot();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      virtual void dump(FILE*) const=0;
    public:
      void edit_xrange(bool);
    private:
      void _layout();
    signals:
      void redraw();
      void counts_changed(double);
    public slots:
      void save_data();
      void set_plot_title();
      void set_xaxis_title();
      void set_yaxis_title();
      void toggle_grid();
      void toggle_minor_grid();
      void xrange_change();
      void yrange_change();
      void update_counts(double);
    public:
      QString      _name;
      QwtPlot*     _frame;
      QLabel*      _counts;
    private:
      AxisControl* _xrange;
      AxisControl* _yrange;
      QwtPlotGrid* _grid;
    };
  };
};

#endif
		 
