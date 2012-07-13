#ifndef AmiQt_AxisBins_hh
#define AmiQt_AxisBins_hh

#include "AxisInfo.hh"

namespace Ami {
  namespace Qt {
    class AxisBins : public AxisInfo {
    public:
      AxisBins(double xlo, double xhi, int n) : _xlo(xlo), _xhi(xhi), _n(n) {}
      ~AxisBins() {}
    public:
      int    lo      () const { return 0; }
      int    hi      () const { return _n; }
      int    center  () const { return _n/2; }
      int    ticks   () const { return _n; }
      int    tick    (double p ) const;
      int    tick_u  (double p ) const;
      double position(int    i) const { return (_xlo*double(_n-i) + _xhi*double(i))/double(_n); }
    public:
      void update(double xlo, double xhi, int n) { _xlo=xlo; _xhi=xhi; _n=n; }
      void update(const AxisBins& a) { _xlo=a._xlo; _xhi=a._xhi; _n=a._n; }
    private:
      double        _xlo, _xhi;
      int           _n;
    };
  };
};

#endif
