#ifndef AmiQt_EdgePlot_hh
#define AmiQt_EdgePlot_hh

#include "ami/qt/QtPlot.hh"

#include <QtCore/QString>

#include <list>

namespace Ami {
  class Cds;
  class DescEntry;
  class EdgeFinder;
  namespace Qt {
    class AxisInfo;
    class ChannelDefinition;
    class CursorDefinition;
    class QtBase;
    class EdgePlot : public QtPlot {
      Q_OBJECT
    public:
      EdgePlot(QWidget*         parent,
	       const QString&   name,
	       unsigned         channel,
	       Ami::EdgeFinder* finder);
      EdgePlot(QWidget*         parent,
	       const char*&     p);
      ~EdgePlot();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels,
		     const AxisInfo&);
      void setup_payload(Cds&);
      void update();
      void dump(FILE*) const;
    private:
      unsigned    _channel;
      Ami::EdgeFinder* _finder;
      unsigned _output_signature;
      QtBase*  _plot;
    };
  };
};

#endif
		 
