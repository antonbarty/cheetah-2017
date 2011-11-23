#ifndef Camera_TwoDGaussianV1_hh
#define Camera_TwoDGaussianV1_hh

#include <inttypes.h>

namespace Pds {
  namespace Camera {
    class TwoDGaussianV1 {
    public:
      enum { Version=1 };
      
      TwoDGaussianV1();
      TwoDGaussianV1(uint64_t n,
		     double xmean,
		     double ymean,
		     double major_ax_width,
		     double minor_ax_width,
		     double tilt);
      ~TwoDGaussianV1();
    public:
      uint64_t integral() const;
      double xmean() const;
      double ymean() const;
      double major_axis_width() const;
      double minor_axis_width() const;
      double major_axis_tilt () const;
    private:
      uint64_t _integral;
      double _xmean;
      double _ymean;
      double _major_axis_width;
      double _minor_axis_width;
      double _major_axis_tilt;
    };
  };
};
#endif
