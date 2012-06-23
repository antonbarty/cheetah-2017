//
//  Class for rectangular frame data
//
#ifndef Pds_ElementHeader_hh
#define Pds_ElementHeader_hh

#include <stdint.h>

namespace Pds {

  namespace Xamps {

    class ElementHeader {
    public:
      ElementHeader();
      enum frameTypes {FrameTypeDarkImage=0, FrameTypeImage=1};
    public:
      unsigned virtual_channel    () const;
      unsigned lane               () const;
      unsigned tid                () const;
      unsigned acq_count          () const;
      unsigned op_code            () const;
      unsigned elementId          () const;
      unsigned seq_count          () const;
//      unsigned ticks              () const;
//      unsigned fiducials          () const;
//      unsigned sb_temp            (unsigned i) const;
      unsigned frame_type         () const;

    private:
      uint32_t _word0;
      uint32_t _word1;
      uint32_t _seq_count;
      uint32_t _ticks;
      uint32_t _fiducials;
      uint16_t _sbtemp[4];
      uint32_t _frame_type;
    };
  };
};
#endif
