/*
 * ASIC_V1.cc
 *
 *  Created on: May 31, 2011
 *      Author: jackp
 */

#include "pdsdata/fexamp/ASIC_V1.hh"
#include <stdio.h>
#include <string.h>

using namespace Pds::Fexamp;

class ASIC_RegistersV1 {
  public:
    ASIC_RegistersV1() {};
    ~ASIC_RegistersV1() {};
    uint32_t addr;
    uint32_t shift;
    uint32_t mask;
    uint32_t defaultValue;
};

static uint32_t _ASICfoo[][4] = {
  //addr shift  mask    default
    { 0,    0,  0x3ff,    0},    // ManualPulseDAC
    { 0,   16,  0x3ff,    1023}, // ThresholdDAC
    { 0,   28,    0x3,    0},    // BaselineAdjust
    { 1,    0,    0x3,    3},    // ResetTime
    { 1,    4,    0xf,    0},    // PumpLength
    { 1,    8,    0x7,    7},    // FilterTimeToFlatTop
    { 1,   12,      1,    0},    // EnableDacMonitor
    { 1,   16,    0x7,    7},    // ResetTweakOP
    { 1,   20,      1,    1},    // ResetCompensation
    { 2,    0,      1,    0},    // TestPulsePolarity
    { 2,    4,      1,    0},    // DisableOutputs
    { 2,    8,      1,    0},    // AutoTestMode
    { 2,   12,      1,    0},    // EnableAPSMon
    { 2,   16,      1,    1},    // Gain
    { 2,   20,      1,    0},    // HighResTestMode
    { 2,   24,      1,    0},    // CalibrationRange
    { 2,   28,      1,    1},    // OutputBuffersEnable
    { 3,    0,      1,    0},    // TestPulserEnable
    { 3,    4,      1,    1},    // EnableAuxiliaryOutput
    { 3,    8,      1,    0},    // DisableMultipleFirings
    { 3,   12,      1,    0},    // DisableFilterPump
    { 3,   16,      1,    0},    // DACMonitorSelect
    { 3,   20,      1,    0},    // SelectCDSTest
    { 3,   24,      1,    0},    // SignalPolarity
    { 3,   28,      1,    0}     // PerampCurrentBooster
};

static ASIC_RegistersV1* _Aregs = (ASIC_RegistersV1*) _ASICfoo;

static bool           namesAreInitialized = false;

ASIC_V1::ASIC_V1() {
  if (!namesAreInitialized){
    int e;
    for (e=0; e< (int)ASIC_V1::NumberOfASIC_Entries; e++) {
      name((ASIC_V1::ASIC_Entries)e, true);
    }
    namesAreInitialized = true;
  }
}

uint32_t   ASIC_V1::get (ASIC_V1::ASIC_Entries e) {
  if (e >= ASIC_V1::NumberOfASIC_Entries) {
    printf("ASIC_V1::get parameter out of range!! %u\n", e);
    return 0;
  }
  return ((_values[_Aregs[e].addr] >> _Aregs[e].shift) & _Aregs[e].mask);
}

const uint32_t ASIC_V1::get (ASIC_V1::ASIC_Entries e) const {
  if (e >= ASIC_V1::NumberOfASIC_Entries) {
    printf("ASIC_V1::get parameter out of range!! %u\n", e);
    return 0;
  }
  return ((_values[_Aregs[e].addr] >> _Aregs[e].shift) & _Aregs[e].mask);
}

uint32_t   ASIC_V1::set (ASIC_V1::ASIC_Entries e, uint32_t v) {
  if (e >= ASIC_V1::NumberOfASIC_Entries) {
    printf("ASIC_V1::set parameter out of range!! %u\n", e);
    return 0;
  }
  _values[_Aregs[e].addr] &= ~(_Aregs[e].mask << _Aregs[e].shift);
  _values[_Aregs[e].addr] |= (_Aregs[e].mask & v) << _Aregs[e].shift;
  return 0;
}
uint32_t   ASIC_V1::rangeHigh(ASIC_V1::ASIC_Entries e) {
  if (e >= ASIC_V1::NumberOfASIC_Entries) {
    printf("ASIC_V1::rangeHigh parameter out of range!! %u\n", e);
    return 0;
  }
  return _Aregs[e].mask;
}
uint32_t   ASIC_V1::rangeLow(ASIC_V1::ASIC_Entries) {
  return 0;
}
uint32_t   ASIC_V1::defaultValue(ASIC_V1::ASIC_Entries e) {
  if (e >= ASIC_V1::NumberOfASIC_Entries) {
    printf("ASIC_V1::defaultValue parameter out of range!! %u\n", e);
    return 0;
  }
  return _Aregs[e].defaultValue & _Aregs[e].mask;
}
char* ASIC_V1::name(ASIC_V1::ASIC_Entries e, bool init) {
  static char _regNames[NumberOfASIC_Entries+1][120] = {
     {"Manual Pulse DAC        "},       //    ManualPulseDAC,
     {"Threshold DAC           "},       //    ThresholdDAC,
     {"Baseline Adjust         "},       //    BaselineAdjust,
     {"Reset Time              "},       //    ResetTime,
     {"Pump Length             "},       //    PumpLength,
     {"Filter Time To Flat Top "},       //    FilterTimeToFlatTop,
     {"Enable Dac Monitor      "},       //    EnableDacMonitor,
     {"Reset Tweak OP          "},       //    ResetTweakOP,
     {"Reset Compensation      "},       //    ResetCompensation,
     {"Test Pulse Polarity     "},       //    TestPulsePolarity,
     {"Disable Outputs         "},       //    DisableOutputs,
     {"Auto Test Mode          "},       //    AutoTestMode,
     {"Enable APS Mon          "},       //    EnableAPSMon,
     {"Gain                    "},       //    Gain,
     {"High Res Test Mode      "},       //    HighResTestMode,
     {"Calibration Range       "},       //    CalibrationRange,
     {"Output Buffers Enable   "},       //    OutputBuffersEnable,
     {"Test Pulser Enable      "},       //    TestPulserEnable,
     {"Enable Auxiliary Output "},       //    EnableAuxiliaryOutput,
     {"Disable Multiple Firings"},       //    DisableMultipleFirings,
     {"Disable Filter Pump     "},       //    DisableFilterPump,
     {"DAC Monitor Select      "},       //    DACMonitorSelect,
     {"Select CDS Test         "},       //    SelectCDSTest,
     {"Signal Polarity         "},       //    SignalPolarity,
     {"Preamp Current Booster  "},       //    PerampCurrentBooster,
     {"---------INVALID--------"}        //    NumberOfASIC_Entries
  };
  static char range[60];
  if (init && (e < ASIC_V1::NumberOfASIC_Entries)) {
    sprintf(range, "  (%u..%u)    ", 0, _Aregs[e].mask);
    strncat(_regNames[e], range, 40);
  }
  return e < ASIC_V1::NumberOfASIC_Entries ? _regNames[e] : _regNames[ASIC_V1::NumberOfASIC_Entries];
}

void ASIC_V1::operator=(ASIC_V1& foo) {
  unsigned i=0;
  while (i<NumberOfASIC_Entries) {
    ASIC_Entries c = (ASIC_Entries)i++;
    set(c,foo.get(c));
  }
  i=0;
  while (i<NumberOfChannels) {
    _channels[i] = foo.channels()[i];
    i += 1;
  }
}

bool ASIC_V1::operator==(ASIC_V1& foo) {
  unsigned i=0;
  bool ret = true;
  while (i<NumberOfASIC_Entries && ret) {
    ASIC_Entries c = (ASIC_Entries)i++;
    ret = (get(c) == foo.get(c));
    if (!ret) printf("\tASIC_V1 %u != %u at %s\n", get(c), foo.get(c), ASIC_V1::name(c));
  }
  i=0;
  while (i<NumberOfChannels && ret) {
    ret = _channels[i] == foo.channels()[i];
    i += 1;
  }
  return ret;
}
