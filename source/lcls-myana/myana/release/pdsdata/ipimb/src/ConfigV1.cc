#include "pdsdata/ipimb/ConfigV1.hh"
#include <stdio.h>

using namespace Pds;

using namespace Ipimb;

static const unsigned Version=1;

Pds::TypeId ConfigV1::typeId() {
  return Pds::TypeId(Pds::TypeId::Id_IpimbConfig,Version);
}

ConfigV1::ConfigV1() {}

ConfigV1::~ConfigV1() {}

void ConfigV1::dump() const {
  printf("------Ipimb Config-------------\n");
  printf("Trigger counter 0x%llx\n", (long long) triggerCounter());
  printf("Serial ID 0x%llx\n", (long long) serialID());
  printf("Charge range %d\n", chargeAmpRange());
  printf("Reset length %ld ns\n", (unsigned long)resetLength());
  printf("Reset delay %d ns\n", resetDelay());
  printf("Reference voltage %f V\n", chargeAmpRefVoltage());
  printf("Diode bias %f V\n", diodeBias());
  printf("Sample delay %ld ns\n", (unsigned long)trigDelay());
}

ConfigV1::ConfigV1 (uint16_t chargeAmpRange,
                    uint16_t calibrationRange,
                    uint32_t resetLength,
                    uint16_t resetDelay,
                    float chargeAmpRefVoltage,
                    float calibrationVoltage,
                    float diodeBias,
                    uint16_t calStrobeLength,
                    uint32_t trigDelay) :
  _triggerCounter(0),
  _serialID(0),
  _chargeAmpRange(chargeAmpRange),
  _calibrationRange(calibrationRange),
  _resetLength(resetLength),
  _resetDelay(resetDelay),
  _chargeAmpRefVoltage(chargeAmpRefVoltage),
  _calibrationVoltage(calibrationVoltage),
  _diodeBias(diodeBias),
  _status(0),
  _errors(0),
  _calStrobeLength(calStrobeLength),
  _trigDelay(trigDelay)
{
}

uint64_t ConfigV1::triggerCounter() const{return _triggerCounter;}
uint64_t ConfigV1::serialID() const{return _serialID;}
uint16_t ConfigV1::chargeAmpRange() const{return _chargeAmpRange;}
uint16_t ConfigV1::calibrationRange() const{return _calibrationRange;}
uint32_t ConfigV1::resetLength() const{return _resetLength;}
uint16_t ConfigV1::resetDelay() const{return _resetDelay;}
float ConfigV1::chargeAmpRefVoltage() const{return _chargeAmpRefVoltage;}
float ConfigV1::calibrationVoltage() const{return _calibrationVoltage;}
float ConfigV1::diodeBias() const{return _diodeBias;}
uint16_t ConfigV1::status() const{return _status;}
uint16_t ConfigV1::errors() const{return _errors;}
uint16_t ConfigV1::calStrobeLength() const{return _calStrobeLength;}
uint32_t ConfigV1::trigDelay() const{return _trigDelay;}

void ConfigV1::setSerialID(uint64_t serialID) {_serialID = serialID;}
void ConfigV1::setErrors(uint16_t errors) {_errors = errors;}
void ConfigV1::setStatus(uint16_t status) {_status = status;}
