#ifndef AmiQt_TdcPlot_hh
#define AmiQt_TdcPlot_hh

#include "ami/qt/QtPlot.hh"
#include <QtCore/QString>

#include "ami/data/ConfigureRequest.hh"

#include <list>

namespace Ami {
  class AbsFilter;
  class Cds;
  class DescEntry;
  namespace Qt {
    class AxisArray;
    class ChannelDefinition;
    class TdcDefinition;
    class QtBase;
    class TdcPlot : public QtPlot {
      Q_OBJECT
    public:
      TdcPlot(QWidget*,
	      const QString&  name,
	      const Ami::AbsFilter& filter,
	      DescEntry*      desc,
	      const QString&  expr);
      TdcPlot(QWidget*,const char*&);
      ~TdcPlot();
    public:
      void save(char*&) const;
      void load(const char*&);
    public:
      void configure(char*& p, unsigned input, unsigned& output);
      void setup_payload(Cds&);
      void update();
      void dump(FILE*) const;
    private:
      Ami::AbsFilter* _filter;
      DescEntry* _desc;
      QString    _expr;

      unsigned _output_signature;

      QtBase*  _plot;
    };
  };
};

#endif
		 
