//
//  Class for rectangular frame data
//
#ifndef Pds_ElementV2_hh
#define Pds_ElementV2_hh

#include "ElementHeader.hh"

namespace Pds {

  namespace CsPad {

    class ElementV2 : public ElementHeader {
    public:
      enum {Version=2};
      ElementV2();
    };
  };
};
#endif
