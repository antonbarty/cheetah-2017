#ifndef Ami_Single_hh
#define Ami_Single_hh

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class DescEntry;
  class Entry;
  class FeatureCache;
  class Term;

  class Single : public AbsOperator {
  public:
    Single(const char* =0);
    Single(const char*&, const DescEntry&, FeatureCache&);
    ~Single();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { SCALE_LEN=256 };
    char           _scale_buffer[SCALE_LEN];
    Entry*         _entry;
    Term*          _term ;
  };

};

#endif
