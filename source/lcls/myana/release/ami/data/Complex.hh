#ifndef COMPLEX_HH
#define COMPLEX_HH
//
//  odfComplex
//
//    A subset of complex number manipulations
//

#include <math.h>

namespace Ami {

  class Complex {
  public:
    Complex() : _realPart(0), _imagPart(0) {}
    Complex(float r, float i) : _realPart(r), _imagPart(i) {}
    Complex(const Complex& c) : _realPart(c._realPart), _imagPart(c._imagPart) {}
    ~Complex() {}

    Complex operator++() { _realPart++; return *this; }
    Complex operator+=(const Complex& c) {
      _realPart+=c._realPart;
      _imagPart+=c._imagPart;
      return *this;
    }
    Complex operator-=(const Complex& c) {
      _realPart-=c._realPart;
      _imagPart-=c._imagPart;
      return *this;
    }
    Complex operator*=(float c) {
      _realPart*=c;
      _imagPart*=c;
      return *this;
    }
    Complex operator*=(const Complex& c) {
      float re(_realPart*c._realPart - _imagPart*c._imagPart);
      float im(_realPart*c._imagPart + _imagPart*c._realPart);
      _realPart=re;
      _imagPart=im;
      return *this;
    }

    float real() const { return _realPart; }
    float imag() const { return _imagPart; }
    float absSq() const { return pow(_realPart,2)+pow(_imagPart,2); }
    float phase() const { return atan2(_imagPart,_realPart); }

    static void transform(Complex*, unsigned, int isign=1);

  private:
    float _realPart, _imagPart;
  };
};

static inline Ami::Complex operator-(const Ami::Complex& a,const Ami::Complex& b) 
{
  Ami::Complex c(a);
  c -= b;
  return c;
}

static inline Ami::Complex operator*(const Ami::Complex& a,const Ami::Complex& b)
{
  Ami::Complex c(a);
  c *= b;
  return c;
}

static inline Ami::Complex operator*(float a,const Ami::Complex& b)
{
  Ami::Complex c(b);
  c *= a;
  return c;
}

static inline void swap(Ami::Complex& a, Ami::Complex& b)
{
  Ami::Complex c(a);
  a = b;
  b = c;
}

#endif

