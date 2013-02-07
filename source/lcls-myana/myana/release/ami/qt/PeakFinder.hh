#ifndef AmiQt_PeakFinder_hh
#define AmiQt_PeakFinder_hh

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>

class QCheckBox;

#include <list>

namespace Ami {

  class AbsOperator;
  class Cds;
  class Entry;
  class DescEntry;

  namespace Qt {
    class ChannelDefinition;
    class ImageScale;
    class PeakPlot;
    class DescImage;
    class ImageDisplay;

    class PeakFinder : public QtPWidget {
      Q_OBJECT
    public:
      PeakFinder(QWidget* parent,
		 ChannelDefinition* channels[], unsigned nchannels, ImageDisplay&);
      ~PeakFinder();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      void save_plots(const QString&) const;
    public:
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels);
      void setup_payload(Cds&);
      void update();
      void prototype(const DescEntry&);
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

      ImageDisplay&  _frame;
      ImageScale*    _threshold;
      QCheckBox*     _accumulate;

      std::list<PeakPlot*> _plots;
    };
  };
};

#endif
