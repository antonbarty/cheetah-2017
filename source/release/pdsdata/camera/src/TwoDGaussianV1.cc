#include "pdsdata/camera/TwoDGaussianV1.hh"

#include <math.h>

using namespace Pds;
using namespace Camera;

TwoDGaussianV1::TwoDGaussianV1() {}
  
TwoDGaussianV1::TwoDGaussianV1(uint64_t n,
			       double xmean,
			       double ymean,
			       double major_ax_width,
			       double minor_ax_width,
			       double tilt) :
  _integral        (n),
  _xmean           (xmean),
  _ymean           (ymean),
  _major_axis_width(major_ax_width),
  _minor_axis_width(minor_ax_width),
  _major_axis_tilt (tilt)
{
}

TwoDGaussianV1::~TwoDGaussianV1() {}

uint64_t TwoDGaussianV1::integral() const { return _integral; }
double TwoDGaussianV1::xmean() const { return _xmean; }
double TwoDGaussianV1::ymean() const { return _ymean; }
double TwoDGaussianV1::major_axis_width() const { return _major_axis_width; }
double TwoDGaussianV1::minor_axis_width() const { return _minor_axis_width; }
double TwoDGaussianV1::major_axis_tilt () const { return _major_axis_tilt; }
