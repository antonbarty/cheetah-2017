#ifndef AmiQt_PeakFitPlot_hh
#define AmiQt_PeakFitPlot_hh

#include "ami/qt/QtPlot.hh"

#include <QtCore/QString>

#include "ami/data/PeakFitPlot.hh"
#include "ami/data/ConfigureRequest.hh"

#include <list>

class QwtPlot;

namespace Ami {
  class Cds;
  class DescEntry;
  namespace Qt {
    class AxisInfo;
    class ChannelDefinition;
    class QtBase;
    class PeakFitPlot : public QtPlot {
      Q_OBJECT
    public:
      PeakFitPlot(QWidget*       parent,
		  const QString& name,
		  unsigned       channel,
		  Ami::PeakFitPlot* desc);
      PeakFitPlot(QWidget*       parent,
		  const char*&   p);
      ~PeakFitPlot();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels,
		     const AxisInfo&, ConfigureRequest::Source);
      void setup_payload(Cds&);
      void update();
      void dump(FILE*) const;
    private:
      unsigned   _channel;
      Ami::PeakFitPlot* _input;
      
      unsigned _output_signature;

      QtBase*  _plot;
    };
  };
};

#endif
		 
