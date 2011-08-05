#ifndef Ami_AbsTransform_hh
#define Ami_AbsTransform_hh

namespace Ami {
  class AbsTransform {
  public:
    virtual ~AbsTransform() {}
  public:
    virtual double operator()(double) const = 0;
  };
};

#endif
