#ifndef CMATRIX_HH
#define CMATRIX_HH

#include "RMatrix.hh"
#include <complex>

typedef std::complex<Real> Complex;

class CMatrix : public Matrix<Complex> {
public:
  CMatrix(int npx, int npy) : Matrix<Complex>(npx, npy) {}
  CMatrix(char *filename);
  CMatrix(RMatrix& m);
  int save(char *filename);
  void print(char *title);
  void fftshift();
  void fft2();
  void ifft2();
};
#endif // CMATRIX_HH
