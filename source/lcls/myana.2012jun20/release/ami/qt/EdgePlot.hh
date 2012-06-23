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
      void savefinder(Ami::EdgeFinder *f, char*& p) const;
      Ami::EdgeFinder *loadfinder(const char *& p);
      void save(char*& p) const;
      void load(const char*& p);
    public:
      void addfinder(Ami::EdgeFinder *f);
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels,
		     const AxisInfo&);
      void setup_payload(Cds&);
      void update();
      void dump(FILE* f, int idx) const;
      void dump(FILE* f) const;
    private:
      unsigned    _channel;
#define MAX_FINDERS  2
      Ami::EdgeFinder* _finder[MAX_FINDERS];
      int      _fcnt;
      unsigned _output_signature;
      QtBase*  _plot[MAX_FINDERS];
      QColor   &getcolor(int i);
    };
  };
};

#endif
		 
