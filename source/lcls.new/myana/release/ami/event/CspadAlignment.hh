#ifndef Ami_CspadAlignment_hh
#define Ami_CspadAlignment_hh

#include <stdio.h>

//
//  Class to convert optical alignment measurements for display
//  Optical alignment coordinates assume device in quadrant 1 position
//

namespace Ami {
  namespace Cspad {

    class TwoByTwoAlignment {
    public:					
      double xOrigin; // with respect to quad origin
      double yOrigin; // with respect to quad origin
      double xAsicOrigin[4]; // with respect to two by two origin
      double yAsicOrigin[4]; // with respect to two by two origin
    };

    class QuadAlignment {
    public:
      TwoByTwoAlignment twobytwo(unsigned) const;
    public:
      struct { 
	struct { double x,y; } _corner[4];
	struct { double x,y; } _pad;
      } _twobyone[8];
    public:
      static QuadAlignment* load(FILE*);
    };
  }
}

#endif
