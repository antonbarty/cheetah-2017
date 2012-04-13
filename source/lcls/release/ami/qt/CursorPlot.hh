#ifndef AmiQt_CursorPlot_hh
#define AmiQt_CursorPlot_hh

#include "ami/qt/QtPlot.hh"

#include <QtCore/QString>

#include "ami/data/BinMath.hh"
#include "ami/data/ConfigureRequest.hh"

#include <list>

namespace Ami {
  class Cds;
  class DescEntry;
  namespace Qt {
    class AxisInfo;
    class ChannelDefinition;
    class CursorDefinition;
    class QtBase;
    class CursorPlot : public QtPlot {
      Q_OBJECT
    public:
      CursorPlot(QWidget*       parent,
		 const QString& name,
		 unsigned       channel,
		 BinMath*       desc);
      CursorPlot(QWidget*       parent,
		 const char*&   p);
      ~CursorPlot();
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
      unsigned _channel;
      BinMath* _input;
      unsigned _output_signature;
      QtBase*  _plot;
    };
  };
};

#endif
		 
