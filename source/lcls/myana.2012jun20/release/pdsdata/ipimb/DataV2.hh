#ifndef IPIMBDATADESC_HH
#define IPIMBDATADESC_HH

#include <stdint.h>

#include "pdsdata/xtc/TypeId.hh"
#include "ConfigV2.hh"

namespace Pds {

  namespace Ipimb {
#pragma pack(4)
    class DataV2 {
      static const int foo = 0;
      static const int ipimbAdcRange = 5;
      static const int ipimbAdcSteps = 65536;

    public:
      enum {Version=2};
      uint64_t triggerCounter() const;
      uint16_t config0() const;
      uint16_t config1() const;
      uint16_t config2() const;
      uint16_t channel0() const;
      uint16_t channel1() const;
      uint16_t channel2() const;
      uint16_t channel3() const;
      uint16_t channel0ps() const;
      uint16_t channel1ps() const;
      uint16_t channel2ps() const;
      uint16_t channel3ps() const;
      float channel0Volts() const;
      float channel1Volts() const;
      float channel2Volts() const;
      float channel3Volts() const;
      float channel0psVolts() const;
      float channel1psVolts() const;
      float channel2psVolts() const;
      float channel3psVolts() const;
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
      uint16_t _channel0ps;
      uint16_t _channel1ps;
      uint16_t _channel2ps;
      uint16_t _channel3ps;
      uint16_t _checksum;

      float _ipimbCountsToVolts(uint16_t x) const {return (float)x*(ipimbAdcRange) / (ipimbAdcSteps - 1);} 
    };
#pragma pack()
  }
}

#endif
