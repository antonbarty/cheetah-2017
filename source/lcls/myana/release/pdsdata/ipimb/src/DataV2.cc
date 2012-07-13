#include "pdsdata/ipimb/DataV2.hh"

using namespace Pds;
using namespace Ipimb;

static const unsigned Version=2;

Pds::TypeId DataV2::typeId() {
  return Pds::TypeId(Pds::TypeId::Id_IpimbData,Version);
}

uint64_t DataV2::triggerCounter() const {
  return _triggerCounter;
}

uint16_t DataV2::config0() const {
  return _config0;
}

uint16_t DataV2::config1() const {
  return _config1;
}

uint16_t DataV2::config2() const {
  return _config2;
}

uint16_t DataV2::channel0() const {
  return _channel0;
}

uint16_t DataV2::channel1() const {
  return _channel1;
}

uint16_t DataV2::channel2() const {
  return _channel2;
}

uint16_t DataV2::channel3() const {
  return _channel3;
}

uint16_t DataV2::channel0ps() const {
  return _channel0ps;
}

uint16_t DataV2::channel1ps() const {
  return _channel1ps;
}

uint16_t DataV2::channel2ps() const {
  return _channel2ps;
}

uint16_t DataV2::channel3ps() const {
  return _channel3ps;
}

float DataV2::channel0Volts() const {
  return _ipimbCountsToVolts(_channel0);
}

float DataV2::channel1Volts() const {
  return _ipimbCountsToVolts(_channel1);
}

float DataV2::channel2Volts() const {
  return _ipimbCountsToVolts(_channel2);
}

float DataV2::channel3Volts() const {
  return _ipimbCountsToVolts(_channel3);
}

float DataV2::channel0psVolts() const {
  return _ipimbCountsToVolts(_channel0ps);
}

float DataV2::channel1psVolts() const {
  return _ipimbCountsToVolts(_channel1ps);
}

float DataV2::channel2psVolts() const {
  return _ipimbCountsToVolts(_channel2ps);
}

float DataV2::channel3psVolts() const {
  return _ipimbCountsToVolts(_channel3ps);
}

uint16_t DataV2::checksum() const {
  return _checksum;
}
