#ifndef AmiQt_FeatureCalculator_hh
#define AmiQt_FeatureCalculator_hh

#include "ami/qt/Calculator.hh"

namespace Ami {
  namespace Qt {
    class FeatureCalculator : public Calculator {
    public:
      FeatureCalculator(const QString&     title);
    public:
      QString result();
    };
  };
};
#endif
