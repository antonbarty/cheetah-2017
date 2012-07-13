#ifndef PHASICS_CONFIG_V1_HH
#define PHASICS_CONFIG_V1_HH

#include <stdint.h>

#pragma pack(4)


namespace Pds
{
  namespace Phasics
  {

    class ConfigV1
    {
      public:
        static const int Version               = 1;
        ConfigV1();
        ~ConfigV1() {};

        enum {Width=640, Height=480, Depth=12};

        enum Registers {
          Brightness,
          Exposure,
          Sharpness,
          Gamma,
          Shutter,
          Gain,
          Pan,
          Tilt,
          NumberOfRegisters
        };

        enum {
          NumberOfValues=NumberOfRegisters
        };


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

    };


  } // namespace Phasics

} // namespace Pds 

#pragma pack()

#endif  // PHASICS_CONFIG_V1_HH
