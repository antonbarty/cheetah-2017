#ifndef AmiQt_CurveFit_hh
#define AmiQt_CurveFit_hh

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>
#include <QtCore/QStringList>

class QComboBox;
class QLabel;

#include <list>

namespace Ami {

  class AbsOperator;
  class Cds;
  class DescEntry;
  class Entry;

  namespace Qt {
    class AxisInfo;
    class ChannelDefinition;
    class WaveformDisplay;
    class ScalarPlotDesc;
    class CurveFitPlot;
    class CurveFitPost;

    class CurveFit : public QtPWidget {
      Q_OBJECT
    public:
      CurveFit(QWidget* parent, ChannelDefinition* channels[], unsigned nchannels, WaveformDisplay&);
      ~CurveFit();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      void save_plots(const QString&) const;
    public:
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels);
      void setup_payload(Cds&);
      void update();
      void initialize(const Ami::DescEntry&);
    public slots:
      void load_file();
      void set_channel   (int); // set the source
      void plot          ();   // configure the plot
      void remove_plot   (QObject*);
      void add_post      ();
    signals:
      void changed();
    private:
      ChannelDefinition** _channels;
      unsigned _nchannels;
      unsigned _channel;
      WaveformDisplay&  _frame;
      QString _fname;
      QLabel* _file;
      QComboBox* _outBox;
      ScalarPlotDesc* _scalar_desc;
      std::list<CurveFitPlot*> _plots;
      std::list<CurveFitPost*> _posts;
      static char *_opname[];
    };
  };
};

#endif
