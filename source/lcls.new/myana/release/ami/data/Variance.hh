#ifndef Ami_Variance_hh
#define Ami_Variance_hh

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class Cds;
  class DescEntry;
  class Entry;
  class FeatureCache;
  class Term;

  class Variance : public AbsOperator {
  public:
    Variance(unsigned n=0, const char* p=0);
    Variance(const char*&, const DescEntry&, FeatureCache&);
    ~Variance();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { SCALE_LEN=256 };
    char           _scale_buffer[SCALE_LEN];
    unsigned       _n;
    Entry*         _mom1;
    Entry*         _mom2;
    Entry*         _cache;
    Term*          _term ;
  };

};

#endif
