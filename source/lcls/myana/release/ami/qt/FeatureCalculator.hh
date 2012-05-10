#ifndef AmiQt_FeatureCalculator_hh
#define AmiQt_FeatureCalculator_hh

#include "ami/qt/Calculator.hh"

namespace Ami {
  namespace Qt {
    class FeatureRegistry;
    class FeatureCalculator : public Calculator {
    public:
      FeatureCalculator(const QString&     title, FeatureRegistry&);
    public:
      QString result();
    };
  };
};
#endif
