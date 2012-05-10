#ifndef Ami_Average_hh
#define Ami_Average_hh

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class Cds;
  class DescEntry;
  class Entry;
  class FeatureCache;
  class Term;

  class Average : public AbsOperator {
  public:
    Average(unsigned n=0, const char* p=0);
    Average(const char*&, const DescEntry&, FeatureCache&);
    ~Average();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { SCALE_LEN=256 };
    char           _scale_buffer[SCALE_LEN];
    unsigned       _n;
    Entry*         _entry;
    Entry*         _cache;
    Term*          _term ;
  };

};

#endif
