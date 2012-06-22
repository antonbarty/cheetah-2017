#ifndef Ami_EntryMath_hh
#define Ami_EntryMath_hh

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class Cds;
  class DescEntry;
  class Entry;
  class Term;
  class FeatureCache;

  class EntryMath : public AbsOperator {
  public:
    EntryMath(const char* expr);
    EntryMath(const char*&, const DescEntry&, const Cds&, FeatureCache&);
    ~EntryMath();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { EXPRESSION_LEN = 256 };
    char       _expression[EXPRESSION_LEN];
    Term*      _term;
    Entry*     _entry;
    mutable unsigned   _index;
  };

};

#endif
