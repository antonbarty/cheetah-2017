#ifndef IPIMBDATAV1_HH
#define IPIMBDATAV1_HH

#include <stdint.h>

#include "pdsdata/xtc/TypeId.hh"
#include "ConfigV1.hh"

// macros for converting IPIMB counts to voltages
#define IPIMB_ADC_RANGE 3.3
#define IPIMB_ADC_STEPS 65536
#define IPIMB_COUNTS_TO_VOLTS(x) ((float) (x) * IPIMB_ADC_RANGE) / (IPIMB_ADC_STEPS - 1);

namespace Pds {

  namespace Ipimb {
#pragma pack(4)
    class DataV1 {
    public:
      enum {Version=1};
      uint64_t triggerCounter() const;
      uint16_t config0() const;
      uint16_t config1() const;
      uint16_t config2() const;
      uint16_t channel0() const;
      uint16_t channel1() const;
      uint16_t channel2() const;
      uint16_t channel3() const;
      float channel0Volts() const;
      float channel1Volts() const;
      float channel2Volts() const;
      float channel3Volts() const;
      uint16_t checksum() const;

      static Pds::TypeId typeId();

    private:
      uint64_t _triggerCounter;
      uint16_t _config0;
      uint16_t _config1;
      uint16_t _config2;
      uint16_t _channel0;
      uint16_t _channel1;
      uint16_t _channel2;
      uint16_t _channel3;
      uint16_t _checksum;
    };
#pragma pack()
  }
}

#endif
