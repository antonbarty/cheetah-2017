#ifndef Ami_Average_hh
#define Ami_Average_hh

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class Cds;
  class DescEntry;
  class Entry;

  class Average : public AbsOperator {
  public:
    Average(unsigned n=0);
    Average(const char*&, const DescEntry&);
    ~Average();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    unsigned       _n;
    Entry*         _entry;
    Entry*         _cache;
  };

};

#endif
