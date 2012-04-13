#ifndef AmiQt_EnvPlot_hh
#define AmiQt_EnvPlot_hh

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
    class EnvDefinition;
    class QtBase;
    class EnvPlot : public QtPlot {
      Q_OBJECT
    public:
      EnvPlot(QWidget*,
	      const QString&  name,
	      const Ami::AbsFilter& filter,
	      DescEntry*      desc);
      EnvPlot(QWidget*,const char*&);
      ~EnvPlot();
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
      
      unsigned _output_signature;

      QtBase*  _plot;
    };
  };
};

#endif
		 
