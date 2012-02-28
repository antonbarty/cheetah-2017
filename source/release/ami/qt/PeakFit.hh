#ifndef AmiQt_PeakFit_hh
#define AmiQt_PeakFit_hh

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>
#include <QtCore/QStringList>

class QLineEdit;
class QButtonGroup;
class QVBoxLayout;

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
    class DescTH1F;
    class DescProf;
    class DescScan;
    class DescChart;
    class WaveformDisplay;

    class PeakFit : public QtPWidget {
      Q_OBJECT
    public:
      PeakFit(QWidget* parent, ChannelDefinition* channels[], unsigned nchannels, WaveformDisplay&);
      ~PeakFit();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      void save_plots(const QString&) const;
    public:
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
    signals:
      void changed();
    private:
      ChannelDefinition** _channels;
      unsigned _nchannels;
      unsigned _channel;

      WaveformDisplay&  _frame;
      EdgeCursor* _baseline;
      unsigned   _quantity;

      QLineEdit* _title;
      QButtonGroup* _plot_grp;
      DescTH1F*  _hist;
      DescChart* _vTime;
      DescProf*  _vFeature;
      DescScan*  _vScan;

      std::list<PeakFitPlot*> _plots;
    };
  };
};

#endif
