#ifndef AmiQt_TdcPlot_hh
#define AmiQt_TdcPlot_hh

//=========================================================
//
//  TwoDPlot for the Analysis and Monitoring Implementation
//
//  Filter configures the server-side filtering
//  Math   configures the server-side operations
//
//=========================================================

#include "ami/qt/QtPlot.hh"

#include <QtCore/QString>

namespace Ami {
  class Cds;
  class DescEntry;
  class AbsFilter;
  namespace Qt {
    class QtBase;
    class TdcPlot : public QtPlot {
      Q_OBJECT
    public:
      TdcPlot(QWidget*,
              const QString&,
              const Ami::AbsFilter&  filter,
              DescEntry*       desc,
              const QString&   expr);
      TdcPlot(QWidget*,const char*&);
      ~TdcPlot();
    public:
      virtual void save(char*& p) const;
      virtual void load(const char*& p);
    public:
      void configure(char*& p, 
		     unsigned input, 
		     unsigned& output);
      void setup_payload(Cds&);
      void update();
      void dump(FILE*) const;
    private:
      Ami::AbsFilter*   _filter;
      DescEntry*        _desc;
      QString           _expr;
      unsigned          _output_signature;
      QtBase*           _plot;
    };
  };
};

#endif      
