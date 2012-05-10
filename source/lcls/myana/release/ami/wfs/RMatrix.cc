#include "RMatrix.hh"
#include "Display.hh"

// constructor that loads data from file
RMatrix::RMatrix(char *filename) : Matrix<Real>(0, 0) {
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    perror(filename);
    exit(1);
  }
  fscanf(fp, "%d", &_npx);
  fscanf(fp, "%d", &_npy);
  _data = new Real[_npx * _npy];
  for (int y = 0; y < _npy; y++) {
    for (int x = 0; x < _npx; x++) {
      double r, i;
      fscanf(fp, "%lf\n", &r); // real part
      fscanf(fp, "%lf\n", &i); // imag part (ignored)
      _data[_npy * x + y] = r;
    }
  }
  fclose(fp);
}

// Expand a matrix into a new matrix larger by "factor" in each dimension.
RMatrix RMatrix::stretch(const int factor) {
  RMatrix& m = *this;
  RMatrix stretched(m._npx * factor, m._npy * factor);
  for (int y = 0; y < stretched._npy; y++) {
    for (int x = 0; x < stretched._npx; x++) {
      stretched[x][y] = m[x / factor][y / factor];
    }
  }
  return stretched;
}

// Shrink a matrix into a new matrix smaller by "factor" in each dimension.
// When combining factor*factor cells into one, use the average value.
RMatrix RMatrix::shrink(const int factor) {
  RMatrix& m = *this;
  RMatrix shrunk(m._npx / factor, m._npy / factor);
  const int factor2 = factor * factor;
  for (int y = 0; y < shrunk._npy; y++) {
    for (int x = 0; x < shrunk._npx; x++) {
      Real val = 0;
      for (int i = 0; i < factor; i++) {
        for (int j = 0; j < factor; j++) {
          val += m[factor * x + i][factor * y + j];
        }
      }
      shrunk[x][y] = val / factor2;
    }
  }
  return shrunk;
}

static inline int round_out(Real x) {
  int ix = (int) x;
  if (x == ix) {
    return ix;
  } else if (x > 0) {
    return ix + 1;
  } else {
    return ix - 1;
  }
}

RMatrix RMatrix::rotate(Real theta, const int stretch_factor, const int shrink_factor) {
  const Real sin_theta = sin(theta);
  const Real cos_theta = cos(theta);

  RMatrix m(this->stretch(stretch_factor));

  // These calculations assume theta is in (-pi/2, pi/2)
  int u_min, u_max, v_min, v_max;
  if (theta > 0) {
    u_min = 0;
    v_min = round_out(-m._npx*sin_theta);
    u_max = round_out(m._npy*sin_theta + m._npx*cos_theta);
    v_max = round_out(m._npy*cos_theta);
  } else {
    v_min = 0;
    u_min = round_out(m._npy*sin_theta);
    v_max = round_out(m._npy*cos_theta - m._npx*sin_theta);
    u_max = round_out(m._npx*cos_theta);
  }

  int npu = u_max - u_min;
  int npv = v_max - v_min;
  RMatrix r(npu, npv);

  for (int v = v_min; v < v_max; v++) {
    for (int u = u_min; u < u_max; u++) {
      int x = round_out(u*cos_theta - v*sin_theta);
      int y = round_out(u*sin_theta + v*cos_theta);
      r[u - u_min][v - v_min] = m.safe_get(x, y); // x or y may be out of bounds
    }
  }

  if (shrink_factor == 1) {
    return r;
  } else {
    display("Before shrinking", r);
    printf("Shrinking by %d...\n", shrink_factor);
    return r.shrink(shrink_factor);
  }
}

RMatrix RMatrix::window(const int npx, const int npy) {
  RMatrix& m = *this;
  assert(npx <= m._npx);
  assert(npy <= m._npy);
  RMatrix windowed(npx, npy);
  int x_start = (m._npx -  npx) / 2;
  int y_start = (m._npy -  npy) / 2;
  for (int x = 0; x < npx; x++) {
    for (int y = 0; y < npy; y++) {
      windowed[x][y] = m[x + x_start][y + y_start];
    }
  }
  return windowed;
}

RMatrix RMatrix::window(const double trim_factor) {
  const int npx = (int) (trim_factor * _npx);
  const int npy = (int) (trim_factor * _npy);
  return window(npx, npy);
}

void RMatrix::normalize(const double range) {
  Real max = _data[0];
  Real min = _data[0];
  for (int y = 0; y < _npy; y++) {
    for (int x = 0; x < _npx; x++) {
      Real val = _data[_npy * x + y];
      if (min > val) min = val;
      if (max < val) max = val;
    }
  }
  Real delta = max - min;
  if (delta == 0) {
    delta = 1;
  }
  const Real center = min + delta / 2;
  const Real range_over_delta = range / delta;
  for (int y = 0; y < _npy; y++) {
    for (int x = 0; x < _npx; x++) {
      Real val = _data[_npy * x + y];
      _data[_npy * x + y] = (val - center) * range_over_delta;
    }
  }
}

// Save data to a file
int RMatrix::save(char *filename) {
  FILE* fp = fopen(filename, "w");
  if (fp == NULL) {
    perror(filename);
    exit(1);
  }
  fprintf(fp, "%d\n", _npx);
  fprintf(fp, "%d\n", _npy);
  for (int y = 0; y < _npy; y++) {
    for (int x = 0; x < _npx; x++) {
      Real val = _data[_npy * x + y];
      fprintf(fp, "%.12f\n", val); // real
      fprintf(fp, "%.12f\n", 0.0); // imag (always zero)
    }
  }
  fclose(fp);
}

void RMatrix::print(char *title) {
#if 1
  printf("%s: (%d, %d)\n", title, _npx, _npy);
  for (int y = 0; y < 7; y++) {
    printf("y=%d: ", y);
    for (int x = 0; x < 7; x++) {
      Real val = _data[_npy * x + y];
      printf("%.5f ", val);
    }
    printf("\n");
  }
  for (int y = _npy - 8; y < _npy; y++) {
    printf("y=%d: ", y);
    for (int x = _npx - 8; x < _npx; x++) {
      Real val = _data[_npy * x + y];
      printf("%.5f ", val);
    }
    printf("\n");
  }
#endif
}
