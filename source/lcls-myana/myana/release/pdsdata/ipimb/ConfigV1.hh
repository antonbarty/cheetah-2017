#ifndef IPIMBCONFIGV1_HH
#define IPIMBCONFIGV1_HH

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

namespace Pds {

  namespace Ipimb {
#pragma pack(4)
    class ConfigV1 {
    public:
      enum {Version=1};
      ConfigV1 ();
      ~ConfigV1 ();
      ConfigV1 (uint16_t chargeAmpRange,
                uint16_t calibrationRange,
                uint32_t resetLength,
                uint16_t resetDelay,
                float chargeAmpRefVoltage,
                float calibrationVoltage,
                float diodeBias,
                uint16_t calStrobeLength,
                uint32_t trigDelay
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
      static Pds::TypeId typeId();

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
    };
#pragma pack()
  }
}

#endif
