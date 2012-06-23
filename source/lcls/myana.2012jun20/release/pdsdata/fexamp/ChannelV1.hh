/*
 * ChannelV1.hh
 *
 *  Created on: May 31, 2011
 *      Author: jackp
 */
#include <stdint.h>


#ifndef FEXAMP_CHANNELV1_HH_
#define FEXAMP_CHANNELV1_HH_

namespace Pds {

  namespace Fexamp {

    class ChannelV1 {
      public:
        ChannelV1();
        ~ChannelV1() {};
        enum ChannelBitFields {
          TrimBits,
          EnableTest,
          ChannelMask,
          ChannelSelectorEnable,
          NumberOfChannelBitFields
        };
        uint32_t              get      (ChannelBitFields);
        const uint32_t        get      (ChannelBitFields) const;
        uint32_t              set      (ChannelBitFields, uint32_t);
        static char*          name     (ChannelBitFields, bool init=false);
        static uint32_t       rangeHigh(ChannelBitFields);
        static uint32_t       rangeLow (ChannelBitFields);
        static uint32_t       defaultValue(ChannelBitFields);
        void                  operator=(ChannelV1&);
        bool                  operator==(ChannelV1&);
        bool                  operator!=(ChannelV1& foo) { return !(*this==foo); }

      private:
        uint32_t   _value;
    };

  }

}

#endif /*  FEXAMP_CHANNELV1_HH_ */
