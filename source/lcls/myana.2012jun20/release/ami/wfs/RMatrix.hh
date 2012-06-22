#ifndef RMATRIX_HH
#define RMATRIX_HH

#include "Matrix.hh"

typedef double Real;

class RMatrix : public Matrix<Real> {
public:
  RMatrix(int npx, int npy) : Matrix<Real>(npx, npy) {}
  RMatrix(char *filename);
  RMatrix rotate();
  RMatrix stretch(const int factor);
  RMatrix shrink(const int factor);
  RMatrix rotate(Real theta, const int stretch_factor, const int shrink_factor);
  RMatrix window(const int npx, const int npy);
  RMatrix window(const double trim_factor);
  void normalize(const double range);
  int save(char *filename);
  void print(char *title);
};

#endif // RMATRIX_HH
