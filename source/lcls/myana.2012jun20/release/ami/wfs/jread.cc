// skunks undead secret service
//#include <cmath>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "RMatrix.hh"
#include "CMatrix.hh"
#include "Display.hh"
#include "Hartmann.hh"
#include "Gradient.hh"

//const double phi = slope_phi;
//const double phi = 0.0;
//const double phi = (M_PI / 180.0) * 0.026; // works with 0.026 but not 0.27
//const double phi = (M_PI / 180.0) * 0.030; // works with 0.026 but not 0.27
//const double phi = (M_PI / 180.0) * 10.0;
//const double phi = (M_PI / 180.0) * 5.0;
//const double phi = (M_PI / 180.0) * 0.440;
const double phi = 0.0;

double gettime() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1.e9;
}

RMatrix create_wavefront_FLAT(const int npx, const int npy) {
  RMatrix wavefront(npx, npy);
  for (int y = 0; y < npy; y++) {
    for (int x = 0; x < npx; x++) {
      //wavefront[x][y] = 1.0 + (x + y) / 1000.0;
      wavefront[x][y] = 1.0;
    }
  }
  return wavefront;
}

RMatrix create_wavefront_X(const int npx, const int npy) {
  RMatrix wavefront(npx, npy);
  const int num_spokes = 0;
  for (int y = 0; y < npy; y++) {
    for (int x = 0; x < npx; x++) {
      const Real ux = 1.0 * x / npx - 0.5;
      const Real uy = 1.0 * y / npy - 0.5;
      const Real q = atan2(ux, uy);
      //wavefront[x][y] = sqrt(ux*ux + uy*uy) * cos(num_spokes*q) + 10 * (50/1.e6*x + 177/1.e6*y);
      wavefront[x][y] = sqrt(ux*ux + uy*uy) * cos(2*q);
    }
  }
  return wavefront;
}

RMatrix create_wavefront(const int npx, const int npy) {
  RMatrix wavefront(npx, npy);
  const int num_spokes = 7;
  for (int y = 0; y < npy; y++) {
    for (int x = 0; x < npx; x++) {
      wavefront[x][y] = (sin(15.0 * x / npx) + cos(23.0 * y / npy));
    }
  }
  return wavefront;
}

Gradients get_gradients_from_wavefront(RMatrix& wavefront) {
  const int npx = wavefront.npx();
  const int npy = wavefront.npy();
  Gradients gradients(npx, npy);
  for (int y = 1; y < npy - 1; y++) {
    for (int x = 1; x < npx - 1; x++) {
      gradients.y[x][y] = wavefront[x][y] - wavefront[x][y - 1];
      gradients.x[x][y] = wavefront[x][y] - wavefront[x - 1][y];
      gradients.x_plus_y[x][y] = (wavefront[x][y] - wavefront[x - 1][y - 1]) / sqrt2;
      gradients.x_minus_y[x][y] = -(wavefront[x][y] - wavefront[x + 1][y - 1]) / sqrt2; // XXX is this correct?
    }
  }
  return gradients;
}

static inline int round_to_int(Real r) {
  if (r >= 0) {
    return (int) (r + 0.5);
  } else {
    return (int) (r - 0.5);
  }
}

RMatrix demodulate(CMatrix& image_fft, Real r_uvx, Real r_uvy, int nx, int ny) {
  printf("**********************************************************************\n");
  int uvx = round_to_int(r_uvx);
  int uvy = round_to_int(r_uvy);

  printf("uvx fractional is %.3f\n", fabs(r_uvx - uvx));
  printf("uvy fractional is %.3f\n", fabs(r_uvy - uvy));

  if (fabs(r_uvx - uvx) > 0.02 ||
      fabs(r_uvy - uvy) > 0.02) {
    return RMatrix(0, 0);
  }

  printf("nx=%d image_fft.npx()/2=%d\n", nx, image_fft.npx()/2);
  printf("ny=%d image_fft.npy()/2=%d\n", ny, image_fft.npy()/2);

  const int xlen = 2 * nx;
  const int ylen = 2 * ny;
  int x_center = image_fft.npx()/2 + (int) uvx;
  int y_center = image_fft.npy()/2 + (int) uvy;
  int xstart = x_center - nx;
  int ystart = y_center - ny;

#if 1 // this breaks at phi=0
  double max_value = -1.0;
  int x_max = x_center;
  int y_max = y_center;
  for (int y = 0; y < ylen; y++) {
    for (int x = 0; x < xlen; x++) {
      Complex c = image_fft[x + xstart][y + ystart];
      Real r0 = c.real();
      Real i0 = c.imag();
      Real value = r0*r0 + i0*i0;
      if (max_value < value) {
        max_value = value;
        x_max = x + xstart;
        y_max = y + ystart;
      }
    }
  }
  printf("Center was (%d, %d)... is now (%d, %d)\n", x_center, y_center, x_max, y_max);
  x_center = x_max;
  y_center = y_max;
#endif

  xstart = x_center - nx;
  ystart = y_center - ny;
  CMatrix window(xlen, ylen);
  for (int y = 0; y < ylen; y++) {
    for (int x = 0; x < xlen; x++) {
      Complex c = image_fft[x + xstart][y + ystart];
      window[x][y] = c;
    }
  }
  //window.print("demodulate window");
  //display("demodulate window", window);

  window.fftshift();
  //window.print("window (shifted)");

  window.ifft2();
  //window.print("inverse");

  RMatrix gradient(window.npx(), window.npy());
  for (int y = 0; y < 2*ny; y++) {
    for (int x = 0; x < 2*nx; x++) {
      Complex c = window[x][y];
      gradient[x][y] = atan2(c.imag(), c.real()) / z;
    }
  }
  //gradient.print("gradient");

  return gradient; // XXX .stretch(4);
}

RMatrix reconstruct(Gradients& gradients, int npx, int npy) {
  int nx = gradients.x.npx();
  int ny = gradients.x.npy();
  //gradients.x.print("r - gradients.x");
  //gradients.y.print("r - gradients.y");

  // Scale by the coordinate sampling
  RMatrix gradxs(nx, ny);
  RMatrix gradys(nx, ny);
  for (int y = 0; y < ny; y++) {
    for (int x = 0; x < nx; x++) {
      gradxs[x][y] = gradients.x[x][y] * npx / nx;
      gradys[x][y] = gradients.y[x][y] * npy / ny;
    }
  }
  //gradxs.print("gradxs");
  //gradys.print("gradys");

  RMatrix wavefront(nx, ny);
  for (int d = 0; d < ny / 2; d++) {
    const int y_max = ny/2+d;
    const int y_min = ny/2-d;
    const int x_max = nx/2+d;
    const int x_min = nx/2-d;

    // Extend on the y_min and y_max edges
    if (y_max < ny-1) {
      for (int x = nx/2-d; x < nx/2+d+1; x++) {
        wavefront[x][y_max+1] = wavefront[x][y_max] + gradys[x][y_max+1];
      }
    }
    if (y_min>0) {
      for (int x = nx/2-d; x < nx/2+d+1; x++) {
        wavefront[x][y_min-1] = wavefront[x][y_min] - gradys[x][y_min];
      }
    }
                
    // Extend on the x_min and x_max edges
    if (x_max<nx-1) {
      for (int y = y_min-1; y < y_max+2; y++) {
        if (y < ny) {
          wavefront[x_max+1][y] = wavefront[x_max][y] + gradxs[x_max+1][y];
        }
      }
    }
    if (x_min>0) {
      for (int y = y_min-1; y < y_max+2; y++) {
        if (y < ny) {
          wavefront[x_min-1][y] = wavefront[x_min][y] - gradxs[x_min][y];
        }
      }
    }
  }

  //wavefront.print("reconstructed wavefront");
  return wavefront;
}

int jsim(int argc, char **argv) {
  // must be powers of two?!
  const int npx = 512;
  const int npy = 512;

  RMatrix wavefront = create_wavefront(npx, npy);
  wavefront.normalize(0.001);
  //wavefront.print("Incident wavefront");
  //display("Incident wavefront", wavefront);

  Gradients gradients0 = get_gradients_from_wavefront(wavefront);
  RMatrix image = gradients0.image();
  //image.print("Intensity");

  if (true /*phi != 0.0*/) {
    //RMatrix rotated = image.rotate(phi, 4, 4);
    RMatrix rotated = image.rotate(phi, 1, 1);
    //rotated.print("Intensity (rotated)");

    image = rotated.window(npx, npy);
    //image.print("Intensity (NOT trimmed)");
  }
  //display("image (rotated)", image);

  CMatrix image_fft(image);
  image_fft.fft2();
  //image_fft.print("Intensity FFT, unshifted");

  // Shift zero frequency to center
  image_fft.fftshift();
  //image_fft.print("Intensity FFT, shifted");

  // Inverse FFT around the carrier frequency
  //   ROI around harmonic

  Real sphi_fc = sin(phi) * fc * npy;
  Real cphi_fc = cos(phi) * fc * npx;

  Real ux = cphi_fc;
  Real uy = -sphi_fc;
  Real vx = sphi_fc;
  Real vy = cphi_fc;

  int nx = (int) (0.5 * (1 + npx * fc));
  int ny = (int) (0.5 * (1 + npy * fc));

  Gradients gradients(nx, ny);
  gradients.x         = demodulate(image_fft, ux,    uy,    nx, ny);
#if 0
  if (gradients.x.npx() == 0) {
    return 0;
  }
#endif

  gradients.y         = demodulate(image_fft, vx,    vy,    nx, ny);
#if 0
  if (gradients.y.npx() == 0) {
    return 0;
  }
#endif

  //gradients.x_plus_y  = demodulate(image_fft, ux+vx, uy+vy, nx, ny);
  //gradients.x_minus_y = demodulate(image_fft, ux-vx, uy-vy, nx, ny);

  // Reconstruct the phase map from its gradients
  RMatrix reconstructed_wavefront = reconstruct(gradients, npx, npy);
  RMatrix reconstructed_wavefront_stretched = reconstructed_wavefront.stretch(4);
  reconstructed_wavefront_stretched.print("Reconstructed wavefront");
  display("Reconstructed wavefront", reconstructed_wavefront_stretched);
}

int main(int argc, char **argv) {
#if 0
  for (double theta = 0.0; theta < 45.0; theta += 0.01) {
    phi = (M_PI / 180.0) * theta;
    printf("\n============= theta=%.3f ==============\n", theta);
    jsim(argc, argv);
  }
#else
  jsim(argc, argv);
#endif
}
