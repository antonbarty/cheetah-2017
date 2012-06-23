/*
 * FrameV1.hh
 *
 *  Created on: Nov 6, 2009
 *      Author: jackp
 */

#ifndef FRAMEV1_HH_
#define FRAMEV1_HH_

#include <stdint.h>

namespace Pds {
  namespace PNCCD {

    class ConfigV1;
    class ConfigV2;
    class FrameV1 {
      public:
        enum {Version=1};

        uint32_t specialWord() const;
        uint32_t frameNumber() const;
        uint32_t timeStampHi() const;
        uint32_t timeStampLo() const;

        const uint16_t* data()                   const;
        const FrameV1* next(const ConfigV1& cfg) const;
        unsigned sizeofData(const ConfigV1& cfg) const;
        const FrameV1* next(const ConfigV2& cfg) const;
        unsigned sizeofData(const ConfigV2& cfg) const;

      private:
        uint32_t _specialWord;
        uint32_t _frameNumber;
        uint32_t _timeStampHi;
        uint32_t _timeStampLo;
    };
  }
}

#endif /* FRAMEV1_HH_ */
