#ifndef Ami_BinMath_hh
#define Ami_BinMath_hh

//
//  class BinMath : an operator that performs algebra upon the bin values of an entry
//    and generates a mean value (Scalar), a distribution (TH1F), or a profile (Prof)
//    versus a BLD or PV quantity.
//

#include "ami/data/AbsOperator.hh"

class QChar;

namespace Ami {

  class Cds;
  class DescEntry;
  class Entry;
  class Term;
  class FeatureCache;

  class BinMath : public AbsOperator {
  public:
    //  Defined by the input entry's signature, the output entry's description,
    //    the algebraic expression, and any BLD/PV dependence for profiles.
    BinMath(const DescEntry& output, const char* expr);
    //  Reconstituted from the input serial stream, the BldState and PvState
    //    accessors, and the Cds input entry accessor.
    BinMath(const char*&, const DescEntry& input, FeatureCache&);
    BinMath(const char*&);
    ~BinMath();
  public:
    DescEntry& output   () const;
    const char*        expression() const;
  public:
    static const QChar& integrate();
    static const QChar& moment1  ();
    static const QChar& moment2  ();
    static const QChar& range    ();
    static const double floatPrecision();
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { EXPRESSION_LEN = 256 };
    char             _expression[EXPRESSION_LEN];
    enum { DESC_LEN = 1024 };
    char             _desc_buffer[DESC_LEN];

    FeatureCache* _cache;
    Term*         _term;
    Term*         _fterm;
    mutable const Entry*  _input;
    Entry*        _entry;
  };

};

#endif
