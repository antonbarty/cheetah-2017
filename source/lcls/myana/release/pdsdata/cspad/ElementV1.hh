//
//  Class for rectangular frame data
//
#ifndef Pds_ElementV1_hh
#define Pds_ElementV1_hh

#include "ElementHeader.hh"

#include <stdint.h>

namespace Pds {

  namespace CsPad {

    class ConfigV1;

    class ElementV1 : public ElementHeader {
    public:
      enum {Version=1};
      ElementV1();
    public:
      //  beginning of pixel data
      const uint16_t* data() const;

      //  location of individual pixel datum
      const uint16_t* pixel(unsigned asic, unsigned column, unsigned row) const;
      const ElementV1* next(const ConfigV1&) const;
    };
  };
};
#endif
