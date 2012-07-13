#ifndef AmiQt_PeakFit_hh
#define AmiQt_PeakFit_hh

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>
#include <QtCore/QStringList>

class QLineEdit;
class QRadioButton;
class QButtonGroup;
class QVBoxLayout;
class QComboBox;

#include "ami/qt/Cursors.hh"
#include "ami/data/ConfigureRequest.hh"

#include <list>

namespace Ami {

  class AbsOperator;
  class Cds;
  class Entry;

  namespace Qt {
    class AxisArray;
    class ChannelDefinition;
    class EdgeCursor;
    class PeakFitPlot;
    class PeakFitPost;
    class DescTH1F;
    class DescProf;
    class DescScan;
    class DescChart;
    class WaveformDisplay;
    class CursorLocation;
    class CursorDefinition;

      class PeakFit : public QtPWidget,
		      public Cursors {
      Q_OBJECT
    public:
      PeakFit(QWidget* parent, ChannelDefinition* channels[], unsigned nchannels, WaveformDisplay&);
      ~PeakFit();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      void save_plots(const QString&) const;
    public:
      void remove(CursorDefinition&);
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels,
		     ConfigureRequest::Source);
      void setup_payload(Cds&);
      void update();
    public slots:
      void set_channel(int); // set the source
      void set_quantity(int); // set the parameter
      void plot        ();   // configure the plot
      void remove_plot (QObject*);
      void add_post    ();
      void grab_cursorx();
      void add_cursor  ();
    signals:
      void changed();
      void grabbed();
    public:
      void mousePressEvent  (double, double);
      void mouseMoveEvent   (double, double);
      void mouseReleaseEvent(double, double);
    private:
      enum { MAX_BINS = 10 };
      QStringList     _names;
      ChannelDefinition** _channels;
      unsigned _nchannels;
      unsigned _channel;

      WaveformDisplay&  _frame;
      EdgeCursor* _baseline;
      unsigned   _quantity;

      QLineEdit* _title;
      QButtonGroup* _base_grp;
      QRadioButton* _const_bl;
      QRadioButton* _linear_bl;
      CursorLocation *_lvalue;
      QButtonGroup* _plot_grp;
      DescTH1F*  _hist;
      DescChart* _vTime;
      DescProf*  _vFeature;
      DescScan*  _vScan;

      std::list<PeakFitPlot*> _plots;
      std::list<PeakFitPost*> _posts;
      std::list<CursorDefinition*> _cursors;

      QVBoxLayout*    _clayout;
    };
  };
};

#endif
