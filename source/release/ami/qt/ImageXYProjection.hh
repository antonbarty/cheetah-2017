#ifndef AmiQt_ImageXYProjection_hh
#define AmiQt_ImageXYProjection_hh

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>
#include <QtCore/QStringList>

class QLineEdit;
class QButtonGroup;
class QTabWidget;

#include <list>

namespace Ami {

  class Cds;
  class Entry;

  namespace Qt {
    class ChannelDefinition;
    class RectangleCursors;
    class ImageFrame;
    class ProjectionPlot;
    class CursorPlot;
    class ZoomPlot;
    class XYHistogramPlotDesc;
    class XYProjectionPlotDesc;
    class ScalarPlotDesc;
    class ImageIntegral;

    class ImageXYProjection : public QtPWidget {
      Q_OBJECT
    public:
      ImageXYProjection(QWidget* parent,
			ChannelDefinition* channels[], unsigned nchannels, ImageFrame&);
      ~ImageXYProjection();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      void save_plots(const QString&) const;
    public:
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels);
      void setup_payload(Cds&);
      void update();
    public slots:
      void set_channel(int);
      void update_range();
      void plot        ();   // configure the plot
      void zoom        ();
      void configure_plot();
      void remove_plot (QObject*);
      virtual void setVisible(bool);
    signals:
      void changed();
    private:
      ChannelDefinition** _channels;
      unsigned _nchannels;
      unsigned _channel;

      ImageFrame&       _frame;
      RectangleCursors* _rectangle;

      QLineEdit*    _title;
      QTabWidget*   _plot_tab;

      XYHistogramPlotDesc*  _histogram_plot;
      XYProjectionPlotDesc* _projection_plot;
      //      ScalarPlotDesc*       _integral_plot;
      ImageIntegral*        _integral_plot;

      std::list<ProjectionPlot*> _pplots;
      std::list<CursorPlot*>     _cplots;
      std::list<ZoomPlot*>       _zplots;
    };
  };
};

#endif
