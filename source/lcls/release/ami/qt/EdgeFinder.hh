#ifndef AmiQt_EdgeFinder_hh
#define AmiQt_EdgeFinder_hh

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>
#include <QtCore/QStringList>

class QLineEdit;
class QButtonGroup;
class QVBoxLayout;
class QComboBox;

#include <list>

namespace Ami {

  class AbsOperator;
  class Cds;
  class DescEntry;
  class Entry;

  namespace Qt {
    class AxisInfo;
    class ChannelDefinition;
    class EdgeCursor;
    class EdgePlot;
    class DescTH1F;
    class WaveformDisplay;

    class EdgeFinder : public QtPWidget {
      Q_OBJECT
    public:
      EdgeFinder(QWidget* parent, ChannelDefinition* channels[], unsigned nchannels, WaveformDisplay&);
      ~EdgeFinder();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      void save_plots(const QString&) const;
    public:
      Ami::AbsOperator* math() const;
    public:
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels);
      void setup_payload(Cds&);
      void update();
      void initialize(const Ami::DescEntry&);
    public slots:
      void set_channel   (int); // set the source
      void plot          ();   // configure the plot
      void remove_plot   (QObject*);
    signals:
      void changed();
    private:
      ChannelDefinition** _channels;
      unsigned _nchannels;
      unsigned _channel;

      WaveformDisplay&  _frame;
      EdgeCursor* _baseline;
      EdgeCursor* _threshold;
 
      QLineEdit* _title;
      DescTH1F*  _hist;

      std::list<EdgePlot*> _plots;
    };
  };
};

#endif
