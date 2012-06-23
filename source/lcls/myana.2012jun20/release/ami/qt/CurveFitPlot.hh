#ifndef AmiQt_CurveFitPlot_hh
#define AmiQt_CurveFitPlot_hh

#include "ami/qt/QtPlot.hh"

#include <QtCore/QString>

#include <list>

namespace Ami {
  class Cds;
  class DescEntry;
  class CurveFit;
  namespace Qt {
    class AxisInfo;
    class ChannelDefinition;
    class CursorDefinition;
    class QtBase;
    class CurveFitPlot : public QtPlot {
      Q_OBJECT
    public:
      CurveFitPlot(QWidget*         parent,
                   const QString&   name,
                   unsigned         channel,
                   Ami::CurveFit*   fit);
      CurveFitPlot(QWidget*         parent,
                   const char*&     p);
      ~CurveFitPlot();
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
      void savefit(char*& p) const;
      Ami::CurveFit *loadfit(const char*& p);
    private:
      unsigned    _channel;
      Ami::CurveFit* _fit;
      unsigned _output_signature;
      QtBase*  _plot;
      DescEntry* _desc;
    };
  };
};

#endif
