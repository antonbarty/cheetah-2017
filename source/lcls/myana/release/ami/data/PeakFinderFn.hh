#ifndef Ami_PeakFinderFn_hh
#define Ami_PeakFinderFn_hh

namespace Ami {
  class PeakFinderFn {
  public:
    virtual ~PeakFinderFn() {};
    virtual void   setup(double,double) = 0;
    virtual unsigned value(unsigned,unsigned) const = 0;
    virtual PeakFinderFn* clone() const = 0;
  };
};

#endif
