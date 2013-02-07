#define max(x, y) ((x) > (y) ? (x) : (y))

class Gradients {
public:
  const int npx;
  const int npy;
  RMatrix x;
  RMatrix y;
  RMatrix x_plus_y;
  RMatrix x_minus_y;
  Gradients(int npx_, int npy_) :
    npx(npx_),
    npy(npy_),
    x(RMatrix(npx, npy)),
    y(RMatrix(npx, npy)),
    x_plus_y(RMatrix(npx, npy)),
    x_minus_y(RMatrix(npx, npy))
  {
  }

  // Scale is image.npx / gradient.npx
  RMatrix wavefront(double scale) {
    RMatrix wavefront(npx, npy);
    const int d_max = max(npx/2 + 1, npy/2 + 1);
    for (int d = 0; d < npy/2; d++) {
      const int y_max = npy/2 + d;
      const int y_min = npy/2 - d;
      const int x_max = npx/2 + d;
      const int x_min = npx/2 - d;

      // Extend on the y_min and y_max edges
      if (y_max < npy - 1) {
        for (int x = npx/2 - d; x < npx/2 + d + 1; x++) {
          double val = wavefront[x][y_max] + scale * y[x][y_max+1];
          wavefront[x][y_max+1] = val;
        }
      }
      if (y_min > 0) {
        for (int x = npx/2 - d; x < npx/2 + d + 1; x++) {
          double val = wavefront[x][y_min] - scale * y[x][y_min];
          wavefront[x][y_min-1] = val;
        }
      }
                
      // Extend on the x_min and x_max edges
      if (x_max < npx - 1) {
        for (int y = y_min - 1; y < y_max + 2; y++) {
          if (y < npy) {
            double val = wavefront[x_max][y] + scale * x[x_max+1][y];
            wavefront[x_max+1][y] = val;
          }
        }
      }
      if (x_min > 0) {
        for (int y = y_min - 1; y < y_max + 2; y++) {
          if (y < npy) {
            double val = wavefront[x_min][y] - scale * x[x_min][y];
            wavefront[x_min-1][y] = val;
          }
        }
      }
    }
    return wavefront;
  }

  RMatrix image() {
    RMatrix image(npx, npy);
    for (int iy = 0; iy < npy; iy++) {
      for (int ix = 0; ix < npx; ix++) {
        image[ix][iy] = (cos(wfc*(ix + z*x[ix][iy])) +
                         cos(wfc*(iy + z*y[ix][iy])) +
                         cos(wfc*(ix + iy + z*x_plus_y[ix][iy])) / 2 +
                         cos(wfc*(ix - iy + z*x_minus_y[ix][iy])) / 2 +
                         1) * 0.25;
      }
    }
    return image;
  }
};
