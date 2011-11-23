#ifndef Ami_LogicAnd_hh
#define Ami_LogicAnd_hh

#include "AbsFilter.hh"

namespace Ami {
  class LogicAnd : public AbsFilter {
  public:
    LogicAnd(AbsFilter&,AbsFilter&);
    ~LogicAnd();
  public:
    bool  accept() const;
    AbsFilter* clone() const;
  private:
    void* _serialize(void*) const;
  private:
    AbsFilter& _a;
    AbsFilter& _b;
  };
};

#endif
