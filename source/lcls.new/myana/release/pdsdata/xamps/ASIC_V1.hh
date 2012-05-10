/*
 * ASIC_V1.hh
 *
 *  Created on: May 31, 2011
 *      Author: jackp
 */

#ifndef ASIC_V1_HH_
#define ASIC_V1_HH_

#include <stdint.h>
#include "pdsdata/xamps/ChannelV1.hh"


namespace Pds {
  namespace Xamps {

    class ASIC_V1 {
      public:
        ASIC_V1();
        ~ASIC_V1() {};

        enum {NumberOfChannels=64, NumberOfValues=4};

        enum ASIC_Entries {
          ManualPulseDAC,
          ThresholdDAC,
          BaselineAdjust,
          ResetTime,
          PumpLength,
          FilterTimeToFlatTop,
          EnableDacMonitor,
          ResetTweakOP,
          ResetCompensation,
          TestPulsePolarity,
          DisableOutputs,
          AutoTestMode,
          EnableAPSMon,
          Gain,
          HighResTestMode,
          CalibrationRange,
          OutputBuffersEnable,
          TestPulserEnable,
          EnableAuxiliaryOutput,
          DisableMultipleFirings,
          DisableFilterPump,
          DACMonitorSelect,
          SelectCDSTest,
          SignalPolarity,
          PreampCurrentBooster,
          NumberOfASIC_Entries
        };

        ChannelV1*            channels ()       { return _channels; }
        const ChannelV1*      channels () const { return _channels; }
        uint32_t              get      (ASIC_Entries);
        const uint32_t        get      (ASIC_Entries) const;
        uint32_t              set      (ASIC_Entries, uint32_t);
        static char*          name     (ASIC_Entries, bool init=false);
        static uint32_t       rangeHigh(ASIC_Entries);
        static uint32_t       rangeLow (ASIC_Entries);
        static uint32_t       defaultValue(ASIC_Entries);

      private:
        ChannelV1         _channels[NumberOfChannels];
        uint32_t          _values[NumberOfValues];
    };
  }
}

#endif /* ASIC_V1_HH_ */
