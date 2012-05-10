#ifndef XAMPS_CONFIG_V1_HH
#define XAMPS_CONFIG_V1_HH

#include <stdint.h>
#include "pdsdata/xamps/ASIC_V1.hh"

#pragma pack(4)


namespace Pds
{
  namespace Xamps
  {

    class ConfigV1
    {
      public:
        static const int Version               = 1;
        ConfigV1();
        ~ConfigV1() {};

        enum {
          NumberOfASICs=16
        };

        enum Registers {
          // NB if these are to be included they should be uncommented ConfigV1.cc/hh and XampsConfigurator.cc
          SC_T0,
          SC_T1,
          SC_T2,
          ROWCLK_T0,
          TRANS_GATE_T0,
          MCLK_T0,
          EXPOSURE_TIME,
//        RESERVED,
          NUM_ROWS,
          TESTMODE,
          HV_SETPOINT,
          DET_READOUT_DLY,
          PGP_READOUT_DLY,
          Voff,
          ASIC_APS,
          SW_HI,
          SW_LO,
          DET_CP,
          DET_PIX,
          DET_SS,
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
        uint32_t          _regs[NumberOfRegisters];
        ASIC_V1           _asics[NumberOfASICs];
        uint32_t          _FPGAversion;

    };


  } // namespace Xamps

} // namespace Pds 

#pragma pack()

#endif  // XAMPS_CONFIG_V1_HH
