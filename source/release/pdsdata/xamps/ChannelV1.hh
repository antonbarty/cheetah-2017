/*
 * ChannelV1.hh
 *
 *  Created on: May 31, 2011
 *      Author: jackp
 */
#include <stdint.h>


#ifndef CHANNELV1_HH_
#define CHANNELV1_HH_

namespace Pds {

  namespace Xamps {

    class ChannelV1 {
      public:
        ChannelV1() {};
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
        static char*          name     (ChannelBitFields);
        static uint32_t       rangeHigh(ChannelBitFields);
        static uint32_t       rangeLow (ChannelBitFields);
        static uint32_t       defaultValue(ChannelBitFields);

      private:
        uint32_t   _value;
    };

  }

}

#endif /* CHANNELV1_HH_ */
