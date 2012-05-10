#ifndef MATRIX_HH
#define MATRIX_HH

#include <cmath>
#include <cstring> // bzero
#include <cassert>
#include <cstdlib>
#include <cstdio>

// Store in row-major order (compatible with C and FFTW)
template<class T> class Matrix {
protected:
  void* _mem;
  T *_data;
  int _npx; // aka number of rows
  int _npy; // aka number of columns
public:
  const int inline npx() { return _npx; }
  const int inline npy() { return _npy; }

  Matrix(const int npx, const int npy) : _npx(npx), _npy(npy) {
    // Allocate a single block for both data and refcount
    _mem = operator new(sizeof(long) + sizeof(T) * npx * npy);
    // refcount uses the first portion
    long& refcount = *(long *)_mem;
    refcount = 1;
    // data uses the rest
    _data = (T*) (&refcount + 1);
    bzero(_data, _npx * _npy * sizeof(T));
  }

  // copy constructor
  Matrix(const Matrix& m) : _npx(m._npx), _npy(m._npy), _mem(m._mem), _data(m._data) {
    long& refcount = *(long *)_mem;
    refcount++;
  }

  // assignment operator
  Matrix& operator=(const Matrix& m) {
    // Release existing data
    long& refcount = *(long *)_mem;
    if (--refcount == 0) {
      operator delete(_mem);
    }
    // Overwrite with data from m
    _mem = m._mem;
    _data = m._data;
    _npx = m._npx;
    _npy = m._npy;
    long& new_refcount = *(long *)_mem;
    new_refcount++;
    return *this;
  }

  ~Matrix() {
    long& refcount = *(long *)_mem;
    if (--refcount == 0) {
      operator delete(_mem);
    }
  }

  inline bool contains(int x, int y) {
    return (x >= 0) && (x < _npx) && (y >= 0) && (y < _npy);
  }

  T safe_get(int x, int y) {
    return contains(x, y) ? _data[_npy * x + y] : 0;
  }

  void safe_set(int x, int y, T val) {
    if (contains(x, y)) {
      _data[_npy * x + y] = val;
    }
  }

  T* operator[](const int x) {
    return &_data[_npy * x];
  }
};

#endif // MATRIX_HH
