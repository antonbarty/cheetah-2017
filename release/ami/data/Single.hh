#ifndef Ami_Single_hh
#define Ami_Single_hh

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class DescEntry;
  class Entry;

  class Single : public AbsOperator {
  public:
    Single();
    Single(const char*&, const DescEntry&);
    ~Single();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    Entry*         _entry;
  };

};

#endif
