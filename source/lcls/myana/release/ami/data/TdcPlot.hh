#ifndef Ami_TdcPlot_hh
#define Ami_TdcPlot_hh

//
//  class TdcPlot : an operator that fetches a TDC module
//    and generates a mean value (Scalar), a distribution (TH1F), or a profile (Prof)
//    versus a BLD or PV quantity.
//

#include "ami/data/AbsOperator.hh"
#include "ami/data/DescImage.hh"

namespace Ami {

  class Entry;
  class Term;

  class TdcPlot : public AbsOperator {
  public:
    //  Defined by the output entry's description
    TdcPlot(const DescEntry& output, const char*);
    //  Reconstituted from the input serial stream and the input entry accessor.
    TdcPlot(const char*&, const DescEntry&);
    TdcPlot(const char*&);
    ~TdcPlot();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { EXPRESSION_LEN = 256 };
    char             _expression[EXPRESSION_LEN];
    enum { DESC_LEN = sizeof(DescImage) };
    char             _desc_buffer[DESC_LEN];

    unsigned      _mask;
    Term*         _xterm;
    Term*         _yterm;
    Entry*        _output;
    class TdcVar;
    TdcVar*       _chan[6];
  };

};

#endif
