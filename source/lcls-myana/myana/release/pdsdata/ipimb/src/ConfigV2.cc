#include "pdsdata/ipimb/ConfigV2.hh"
#include <stdio.h>

using namespace Pds;

using namespace Ipimb;

static const unsigned Version=2;
const char* ConfigV2::cap_range[] = { "1 pF", "4.7 pF", "24pF", "120pF", "620 pF", "3.3nF","10 nF", "expert", NULL };

Pds::TypeId ConfigV2::typeId() {
  return Pds::TypeId(Pds::TypeId::Id_IpimbConfig,Version);
}

ConfigV2::ConfigV2() {}

ConfigV2::~ConfigV2() {}

void ConfigV2::dump() const {
  printf("------Ipimb Config-------------\n");
  printf("Trigger counter 0x%llx\n", (long long) triggerCounter());
  printf("Serial ID 0x%llx\n", (long long) serialID());
  printf("Charge amp settings 0x%x\n", chargeAmpRange());
  printf("Reset length %ld ns\n", (unsigned long)resetLength());
  printf("Reset delay %d ns\n", resetDelay());
  printf("Reference voltage %f V\n", chargeAmpRefVoltage());
  printf("Diode bias %f V\n", diodeBias());
  printf("Sample delay %ld ns\n", (unsigned long)trigDelay());
}

ConfigV2::ConfigV2 (uint16_t chargeAmpRange,
                    uint16_t calibrationRange,
                    uint32_t resetLength,
                    uint16_t resetDelay,
                    float chargeAmpRefVoltage,
                    float calibrationVoltage,
                    float diodeBias,
                    uint16_t calStrobeLength,
                    uint32_t trigDelay,
                    uint32_t trigPsDelay,
                    uint32_t adcDelay) :
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
  _trigDelay(trigDelay),
  _trigPsDelay(trigPsDelay),
  _adcDelay(adcDelay)
{
}

uint64_t ConfigV2::triggerCounter() const{return _triggerCounter;}
uint64_t ConfigV2::serialID() const{return _serialID;}
uint16_t ConfigV2::chargeAmpRange() const{return _chargeAmpRange;}
uint16_t ConfigV2::calibrationRange() const{return _calibrationRange;}
uint32_t ConfigV2::resetLength() const{return _resetLength;}
uint16_t ConfigV2::resetDelay() const{return _resetDelay;}
float ConfigV2::chargeAmpRefVoltage() const{return _chargeAmpRefVoltage;}
float ConfigV2::calibrationVoltage() const{return _calibrationVoltage;}
float ConfigV2::diodeBias() const{return _diodeBias;}
uint16_t ConfigV2::status() const{return _status;}
uint16_t ConfigV2::errors() const{return _errors;}
uint16_t ConfigV2::calStrobeLength() const{return _calStrobeLength;}
uint32_t ConfigV2::trigDelay() const{return _trigDelay;}
uint32_t ConfigV2::trigPsDelay() const{return _trigPsDelay;}
uint32_t ConfigV2::adcDelay() const{return _adcDelay;}

void ConfigV2::setTriggerPreSampleDelay(uint32_t trigPsDelay) {_trigPsDelay = trigPsDelay;};
void ConfigV2::setAdcDelay(uint32_t adcDelay) {_adcDelay = adcDelay;};
void ConfigV2::setSerialID(uint64_t serialID) {_serialID = serialID;}
void ConfigV2::setErrors(uint16_t errors) {_errors = errors;}
void ConfigV2::setStatus(uint16_t status) {_status = status;}
