#include "pdsdata/xamps/ConfigV1.hh"
#include <stdio.h>
#include <string.h>

using namespace Pds::Xamps;

class RegisterV1 {
  public:
    RegisterV1() {};
    ~RegisterV1() {};
    uint32_t mask;
    uint32_t defaultValue;
};

static uint32_t _regsfoo[][2] = {
    {    0xFFFF,         30},           //    SC_T0,
    {    0xFFFF,         40},           //    SC_T1,
    {    0xFFFF,         40},           //    SC_T2,
    {    0xFFFF,         90},           //    ROWCLK_T0,
    {    0xFFFF,         31},           //    TRANS_GATE_T0,
    {    0xFFFF,         12},           //    MCLK_T0,
    {    0xFFFFFFFF,     100000},       //    EXPOSURE_TIME,
//    {  0     ,              0},           //    RESERVED,
    {    0x1FFFF,        1024},          //    NUM_ROWS,
    {    1,              0},                //    TESTMODE,
    {    0xFFF,          0},           //    HV_SETPOINT,
    {    0xFFFFFFFF,     0},       //    DET_READOUT_DLY,
    {    0xFFFFFFFF,     0},       //    PGP_READOUT_DLY,
    {    0xFFF,          0},            //    Voff,
    {    0xFFF,          0},            //    ASIC_APS,
    {    0xFFF,          0},            //    SW_HI,
    {    0xFFF,          0},            //    SW_LO,
    {    0xFFF,          0},            //    DET_CP,
    {    0xFFF,          0},            //    DET_PIX,
    {    0xFFF,          0}            //    DET_SS,
                      //    NumberOfRegisters
};

static RegisterV1* _r = (RegisterV1*) _regsfoo;

static bool           namesAreInitialized = false;

ConfigV1::ConfigV1() {
  if (!namesAreInitialized){
    int r;
    for (r=0; r< (int)ConfigV1::NumberOfRegisters; r++) {
      name((ConfigV1::Registers)r, true);
    }
    namesAreInitialized = true;
  }
}

uint32_t            ConfigV1::get      (Registers r) {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::get parameter of of range!! %u\n", r);
    return 0;
  }
  return (_regs[r] & _r[r].mask);
};
const uint32_t      ConfigV1::get      (Registers r) const {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::get parameter of of range!! %u\n", r);
    return 0;
  }
  return (_regs[r] & _r[r].mask);
};
uint32_t            ConfigV1::set      (Registers r, uint32_t v) {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::set parameter of of range!! %u\n", r);
    return 0;
  }
  _regs[r] = v & _r[r].mask;
  return 0;
};
uint32_t            ConfigV1::rangeHigh(Registers r) {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::rangeHigh parameter of of range!! %u\n", r);
    return 0;
  }
  return _r[r].mask;
};
uint32_t            ConfigV1::rangeLow (Registers r) {
  return 0;
};
uint32_t            ConfigV1::defaultValue(Registers r) {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::defaultValue parameter of of range!! %u\n", r);
    return 0;
  }
  return _r[r].defaultValue & _r[r].mask;
}
char*               ConfigV1::name     (Registers r, bool init) {
  static char _regsNames[NumberOfRegisters+1][120] = {
    {"SC_T0          "},  //      SC_T0,
    {"SC_T1          "},  //      SC_T1,
    {"SC_T2          "},  //      SC_T2,
    {"ROWCLK_T0      "},  //      ROWCLK_T0,
    {"TRANS_GATE_T0  "},  //      TRANS_GATE_T0,
    {"MCLK_T0        "},  //      MCLK_T0,
    {"EXPOSURE_TIME  "},  //      EXPOSURE_TIME,
                        ////    RESERVED,
    {"NUM_ROWS       "},  //      NUM_ROWS,
    {"TESTMODE       "},  //      TESTMODE,
    {"HV_SETPOINT    "},  //      HV_SETPOINT,
    {"DET_READOUT_DLY"},  //      DET_READOUT_DLY,
    {"PGP_READOUT_DLY"},  //      PGP_READOUT_DLY,
    {"Voff           "},  //      Voff,
    {"ASIC_APS       "},  //      ASIC_APS,
    {"SW_HI          "},  //      SW_HI,
    {"SW_LO          "},  //      SW_LO,
    {"DET_CP         "},  //      DET_CP,
    {"DET_PIX        "},  //      DET_PIX,
    {"DET_SS         "},  //      DET_SS,
    {"----INVALID----"}   //      NumberOfRegisters
  };

  static char range[60];

  if (init && (r < ConfigV1::NumberOfRegisters)) {
    sprintf(range, "  (%u..%u)    ", 0, _r[r].mask);
    strncat(_regsNames[r], range, 40);
  }

  return r < ConfigV1::NumberOfRegisters ? _regsNames[r] : _regsNames[ConfigV1::NumberOfRegisters];

};

