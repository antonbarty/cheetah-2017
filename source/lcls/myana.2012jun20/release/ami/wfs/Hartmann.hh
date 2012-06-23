#ifndef HARTMANN_HH
#define HARTMANN_HH

// Modified Hartmann Mask
const double slope = 157. / 255;
const double slope_phi = atan(slope);
const double d = 40.e-6; // grid spacing
const double p = 10.e-6; // pixel spacing
const double z = 20.e-3 / p; // distance from mask to screen, normalized by pixel spacing
const double fc = p / d; // carrier frequency
const double wfc = M_PI * 2 * fc; // carrier angular frequency
const double sqrt2 = sqrt(2);

#endif
