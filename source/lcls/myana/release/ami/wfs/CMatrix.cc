#include "CMatrix.hh"
#include "fftw3.h"

CMatrix::CMatrix(RMatrix& m) : Matrix<Complex>(m.npx(), m.npy()) {
  int x, y;
  for (y = 0; y < _npy; y++) {
    for (x = 0; x < _npx; x++) {
      Complex c(m[x][y], 0);
      _data[_npy * x + y] = c;
    }
  }
}

// constructor that loads data from file
CMatrix::CMatrix(char *filename) : Matrix<Complex>(0, 0) {
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    perror(filename);
    exit(1);
  }
  fscanf(fp, "%d", &_npx);
  fscanf(fp, "%d", &_npy);
  _data = new Complex[_npx * _npy];
  int x, y;
  for (y = 0; y < _npy; y++) {
    for (x = 0; x < _npx; x++) {
      double r, i;
      fscanf(fp, "%lf\n", &r); // real part
      fscanf(fp, "%lf\n", &i); // imag part (ignored)
      Complex c(r, i);
      _data[_npy * x + y] = c;
    }
  }
  fclose(fp);
}

// Save data to a file
int CMatrix::save(char *filename) {
  FILE* fp = fopen(filename, "w");
  if (fp == NULL) {
    perror(filename);
    exit(1);
  }
  fprintf(fp, "%d\n", _npx);
  fprintf(fp, "%d\n", _npy);
  int x, y;
  for (y = 0; y < _npy; y++) {
    for (x = 0; x < _npx; x++) {
      Complex val = _data[_npy * x + y];
      fprintf(fp, "%.12f\n", val.real());
      fprintf(fp, "%.12f\n", val.imag());
    }
  }
  fclose(fp);
}

void CMatrix::print(char *title) {
#if 1
  printf("%s: \n", title);
  for (int y = 0; y < 7; y++) {
    printf("y=%d: ", y);
    for (int x = 0; x < 7; x++) {
      Complex c = _data[_npy * x + y];
      printf("(%.2f + %.2fj) ", c.real(), c.imag());
    }
    printf("\n");
  }
#endif
}

// Interchange entries in 4 quadrants, 1 <--> 3 and 2 <--> 4.
// Only works for even x, y.
void CMatrix::fftshift() {
  CMatrix& m = *this;
  const int x2 = m.npx() / 2;
  const int y2 = m.npy() / 2;
  assert(m.npx() == 2 * x2);
  assert(m.npy() == 2 * y2);

  for (int y = 0; y < y2; y++) {
    for (int x = 0; x < x2; x++) {
      swap(m[x][y], m[x + x2][y + y2]);
      swap(m[x + x2][y], m[x][y + y2]);
    }
  }
}

void CMatrix::fft2() {
  CMatrix& m = *this;
  assert(m.npx() == m.npy());
  const int size = m.npx();
  const int n = size * size;
  fftw_complex* data = (fftw_complex*) &m[0][0];
  fftw_plan plan = fftw_plan_dft_2d(size, size, data, data, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(plan);
  fftw_destroy_plan(plan);
}

void CMatrix::ifft2() {
  CMatrix& m = *this;
  const int npx = m.npx();
  const int npy = m.npy();
  fftw_complex* data = (fftw_complex*) &m[0][0];

  const fftw_plan plan = fftw_plan_dft_2d(npx, npy, data, data, FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_execute(plan);
  fftw_destroy_plan(plan);

  const int n = npx * npy;
  for (int y = 0; y < npy; y++) {
    for (int x = 0; x < npx; x++) {
      m[x][y] /= n;
    }
  }
}
