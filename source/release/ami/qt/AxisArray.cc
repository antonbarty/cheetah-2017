#include "AxisArray.hh"

using namespace Ami::Qt;

int    AxisArray::tick    (double p ) const {
  for(int i=1; i<_n; i++) {
    double dxi = p - _x[i];
    double dxim= p - _x[i-1];
    if ((dxi>0 && dxim<=0) || 
	(dxi<=0 && dxim>0))
      return i-1;
  }
  return 0;
}
