#ifndef Ami_Reference_hh
#define Ami_Reference_hh

#include "ami/data/AbsOperator.hh"

namespace Ami {

  class Cds;
  class DescEntry;
  class Entry;

  class Reference : public AbsOperator {
  public:
    Reference(const char* path);
    Reference(const char*&, const DescEntry&);
    ~Reference();
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    enum { PATHLEN=256 };
    char           _path[PATHLEN];
    char           _buffer[4096];
    Entry*         _entry;
  };

};

#endif
