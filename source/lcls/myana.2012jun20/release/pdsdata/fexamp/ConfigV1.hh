#ifndef FEXAMP_CONFIG_V1_HH
#define FEXAMP_CONFIG_V1_HH

#include <stdint.h>
#include "pdsdata/fexamp/ASIC_V1.hh"

#pragma pack(4)


namespace Pds
{
  namespace Fexamp
  {

    class ConfigV1
    {
      public:
        static const int Version               = 1;
        ConfigV1();
        ~ConfigV1() {};

        enum {
          NumberOfASICs=1, NumberOfValues=7
        };

        enum Registers {
          AsicVersion,
          CckDisable,
          MckDisable,
          EnExtTrig,
          LemoSelect,
          NoPayload,
          ClkDisable,
          AsicRstHwEn,
          PtDelay,
          ScDelay,
          CCkPosWidth,
          CCkNegWidth,
          ScPosWidth,
          ScNegWidth,
          ScCount,
          MckPosWidth,
          AdcClkPer,
          MckNegWidth,
          MckLimit,
          MckDelay,
          AdcDelay,
          AdcPhase,
          PerMclkCount,
          SlowAdcDelay0,
          SlowAdcDelay1,
          NumberOfRegisters
        };

        const uint32_t        FPGAversion() const { return _FPGAversion; }
        uint32_t              FPGAversion()       { return _FPGAversion; }
        void                  FPGAVersion(uint32_t v) { _FPGAversion = v; }
        const ASIC_V1*        ASICs() const {return _asics; }
        ASIC_V1*              ASICs() { return _asics; }

        static const int      version() { return Version; }

        uint32_t              get      (Registers);
        const uint32_t        get      (Registers) const;
        uint32_t              set      (Registers, uint32_t);
        static char*          name     (Registers, bool init=false);
        static uint32_t       rangeHigh(Registers);
        static uint32_t       rangeLow (Registers);
        static uint32_t       defaultValue(Registers);


      private:
        uint32_t          _values[NumberOfValues];
        ASIC_V1           _asics[NumberOfASICs];
        uint32_t          _FPGAversion;

    };


  } // namespace Fexamp

} // namespace Pds 

#pragma pack()

#endif  // FEXAMP_CONFIG_V1_HH
