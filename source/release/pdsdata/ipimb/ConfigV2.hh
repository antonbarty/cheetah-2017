#ifndef IPIMBCONFIGV2_HH
#define IPIMBCONFIGV2_HH

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>
#include <stddef.h>

namespace Pds {

  namespace Ipimb {
#pragma pack(4)
    class ConfigV2 {
    public:
      enum {Version=2};
      enum CapacitorValue {c_1pF, c_4p7pF, c_24pF, c_120pF, c_620pF, c_3p3nF, c_10nF, expert};
      static const char* cap_range[];
      ConfigV2 ();
      ~ConfigV2 ();
      ConfigV2 (uint16_t chargeAmpRange,
                uint16_t calibrationRange,
                uint32_t resetLength,
                uint16_t resetDelay,
                float chargeAmpRefVoltage,
                float calibrationVoltage,
                float diodeBias,
                uint16_t calStrobeLength,
                uint32_t trigDelay,
                uint32_t trigPsDelay,
                uint32_t adcDelay
                );
      uint64_t triggerCounter() const;
      uint64_t serialID() const;
      uint16_t chargeAmpRange() const;
      uint16_t calibrationRange() const;
      uint32_t resetLength() const;
      uint16_t resetDelay() const;
      float chargeAmpRefVoltage() const;
      float calibrationVoltage() const;
      float diodeBias() const;
      uint16_t status() const;
      uint16_t errors() const;
      uint16_t calStrobeLength() const;
      uint32_t trigDelay() const;
      uint32_t trigPsDelay() const;
      uint32_t adcDelay() const;
      static Pds::TypeId typeId();

      void setAdcDelay(uint32_t adcDelay);
      void setTriggerPreSampleDelay(uint32_t trigPsDelay);
      void setSerialID(uint64_t serialID);
      void setErrors(uint16_t errors);
      void setStatus(uint16_t status);
      void dump() const;

    private:
      uint64_t _triggerCounter;
      uint64_t _serialID;
      uint16_t _chargeAmpRange;
      uint16_t _calibrationRange;
      uint32_t _resetLength;
      uint32_t _resetDelay; // for alignment use uint32 instead of uint16
      float _chargeAmpRefVoltage;
      float _calibrationVoltage;
      float _diodeBias;
      uint16_t _status;
      uint16_t _errors;
      uint16_t _calStrobeLength;
      uint32_t _trigDelay;
      uint32_t _trigPsDelay;
      uint32_t _adcDelay;
    };
#pragma pack()
  }
}

#endif
