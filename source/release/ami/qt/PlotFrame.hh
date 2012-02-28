#ifndef AmiQt_PlotFrame_hh
#define AmiQt_PlotFrame_hh

#include "ami/qt/CursorTarget.hh"

#include "qwt_plot.h"

namespace Ami {
  namespace Qt {
    class Cursors;
    class PlotFrame : public QwtPlot,
		      public CursorTarget {
      Q_OBJECT
    public:
      PlotFrame(QWidget*);
      ~PlotFrame();
    protected:
      void mousePressEvent(QMouseEvent* e);
      void mouseReleaseEvent(QMouseEvent* e);
    public:
      void set_cursor_input(Cursors* c);
    private:
      Cursors* _c;
    };
  };
};

#endif
