#ifndef AmiQt_NullTransform_hh
#define AmiQt_NullTransform_hh

#include "ami/data/AbsTransform.hh"

namespace Ami {
  namespace Qt {
    class NullTransform : public Ami::AbsTransform {
    public:
      ~NullTransform() {}
      double operator()(double x) const { return x; }
    };
  };
};

#endif
