#ifndef Ami_LogicOr_hh
#define Ami_LogicOr_hh

#include "AbsFilter.hh"

namespace Ami {
  class LogicOr : public AbsFilter {
  public:
    LogicOr(AbsFilter&,AbsFilter&);
    ~LogicOr();
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
