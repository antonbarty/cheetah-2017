#ifndef Ami_Contour_hh
#define Ami_Contour_hh

namespace Ami {

  class Contour {
  public:
    enum { MaxOrder=2 };
    Contour();
    Contour(float*, unsigned order, double discrimLevel);
    Contour(const Contour&);
    ~Contour();
  public:
    double discrimLevel() const;
    float value(float) const;
    void  extremes(double  x0,double  x1,
		   double& y0,double& y1) const;
  private:
    unsigned  _n;
    double    _discrimLevel;
    float     _c[MaxOrder+1];
  };
};

#endif
