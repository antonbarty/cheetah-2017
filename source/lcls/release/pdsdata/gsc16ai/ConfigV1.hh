//
//  Class for configuration of General Standards Corporation 16AI32SSC ADC
//
#ifndef GSC16CONFIGV1_HH
#define GSC16CONFIGV1_HH

#include <stdint.h>

#pragma pack(4)

namespace Pds {

  namespace Gsc16ai {

    class ConfigV1 {
    public:
      enum { Version=1 };
      enum InputMode    { InputMode_Differential=0, InputMode_Zero=1, InputMode_Vref=2 };
      enum VoltageRange { VoltageRange_10V=0, VoltageRange_5V, VoltageRange_2_5V };
      enum TriggerMode  { TriggerMode_ExtPos=0, TriggerMode_ExtNeg, TriggerMode_IntClk };
      enum DataFormat   { DataFormat_TwosComplement=0, DataFormat_OffsetBinary };
      enum { LowestChannel=0, HighestChannel=15 };
      enum { LowestFps=1, HighestFps=120 };

      ConfigV1();
      ConfigV1(
      uint16_t    voltageRange,
      uint16_t    firstChan,
      uint16_t    lastChan,
      bool        autocalibEnable = true,
      uint16_t    inputMode = InputMode_Differential,
      uint16_t    triggerMode = TriggerMode_ExtPos,
      uint16_t    dataFormat = DataFormat_TwosComplement,
      uint16_t    fps = 0,
      bool        timeTagEnable = false
      );

      unsigned          size()              const     { return (unsigned) sizeof(*this); }

      uint16_t          voltageRange()      const     { return _voltageRange; }
      uint16_t          firstChan()         const     { return _firstChan; }
      uint16_t          lastChan()          const     { return _lastChan; }
      uint16_t          inputMode()         const     { return _inputMode; }
      uint16_t          triggerMode()       const     { return _triggerMode; }
      uint16_t          dataFormat()        const     { return _dataFormat; }
      uint16_t          fps()               const     { return _fps; }
      bool              autocalibEnable()   const     { return (_autocalibEnable != 0); }
      bool              timeTagEnable()     const     { return (_timeTagEnable != 0); }

      // misc
      void              dump()              const;

private:
      uint16_t    _voltageRange;
      uint16_t    _firstChan;
      uint16_t    _lastChan;
      uint16_t    _inputMode;
      uint16_t    _triggerMode;
      uint16_t    _dataFormat;
      uint16_t    _fps;
      int8_t      _autocalibEnable;
      int8_t      _timeTagEnable;
    };
  };
};

#pragma pack()

#endif
