#ifndef PNCCD_ConfigV1_hh
#define PNCCD_ConfigV1_hh

#include <stdint.h>

namespace Pds {

  namespace PNCCD {

    class ConfigV1 {
    public:
      enum { Version=1 };
      ConfigV1();
      ConfigV1(uint32_t width, uint32_t height);
      uint32_t numLinks()           const;
      uint32_t payloadSizePerLink() const; // in bytes
      unsigned size()               const;
    private:
      uint32_t _numLinks;
      uint32_t _payloadSizePerLink;
    };

  };
};

#endif
