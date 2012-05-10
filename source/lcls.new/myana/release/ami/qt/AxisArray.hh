#ifndef AmiQt_AxisArray_hh
#define AmiQt_AxisArray_hh

#include "AxisInfo.hh"

namespace Ami {
  namespace Qt {
    class AxisArray : public AxisInfo {
    public:
      AxisArray(const double* x, int n) : _x(x), _n(n) {}  // array must be size n+1
      ~AxisArray() {}
    public:
      int    lo      () const { return 0; }
      int    hi      () const { return _n; }
      int    center  () const { return _n/2; }
      int    ticks   () const { return _n; }
      int    tick    (double p ) const;
      double position(int    i) const { return _x[i]; }
    public:
      void update(const double* x, int n) { _x=x; _n=n; }
      void update(const AxisArray& a) { _x=a._x; _n=a._n; }
    private:
      const double* _x;
      int           _n;
    };
  };
};

#endif
