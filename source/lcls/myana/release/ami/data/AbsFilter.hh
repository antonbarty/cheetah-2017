#ifndef Ami_AbsFilter_hh
#define Ami_AbsFilter_hh

#include <stdint.h>

namespace Ami {
  class AbsFilter {
  public:
    enum Type { Raw,      // simple filters
		SingleShot,
		FeatureRange,
		LogicAnd, // compound filters
		LogicOr };
    AbsFilter(Type t) : _type(t) {}
    virtual ~AbsFilter() {}
  public:
    Type  type() const { return (Type)_type; }
  public:
    virtual bool  accept() const = 0;
    virtual AbsFilter* clone() const = 0;
  public:
    void* serialize(void*) const;
  private:
    virtual void* _serialize(void* p) const = 0;
  protected:
    //  helper functions for (de)serialization
    void _insert (void*& p, const void* b, unsigned size) const;
    void _extract(const char*& p, void* b, unsigned size);
  private:
    uint32_t _type;
  };
};

#endif
