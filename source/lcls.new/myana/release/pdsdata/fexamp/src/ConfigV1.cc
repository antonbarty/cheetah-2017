#include "pdsdata/fexamp/ConfigV1.hh"
#include <stdio.h>
#include <string.h>

using namespace Pds::Fexamp;
//<register> <name>GenControl</name> <address>0</address> <lane>0</lane> <vc>1</vc> <size>1</size>
//   <field> <bits>3:0</bits><label>AsicVersion</label></field>
//   <field> <bits>5</bits><label>CckDisable</label></field>
//   <field> <bits>6</bits><label>MckDisable</label></field>
//   <field> <bits>7</bits><label>EnExtTrig</label></field>
//   <field> <bits>11:8</bits> <label>LemoSelect</label> </field>
//   <field> <bits>12</bits><label>NoPayload</label></field>
//   <field> <bits>13</bits><label>ClkDisable</label></field>
//   <field> <bits>14</bits><label>AsicRstHwEn</label></field>
//</register>
//<register> <name>TimeControl0</name> <address>16</address> <lane>0</lane> <vc>1</vc> <size>1</size>
//   <field> <bits>7:0</bits> <label>PtDelay</label> <comp a="1" b="4" c="0">nS</comp> </field>
//   <field> <bits>15:8</bits> <label>ScDelay</label> <comp a="1" b="4" c="0">nS</comp> </field>
//   <field> <bits>23:16</bits> <label>CCkPosWidth</label> <comp a="1" b="4" c="0">nS</comp> </field>
//   <field> <bits>31:24</bits> <label>CCkNegWidth</label> <comp a="1" b="4" c="0">nS</comp> </field>
//</register>
//<register> <name>TimeControl1</name> <address>17</address> <lane>0</lane> <vc>1</vc> <size>1</size>
//   <field> <bits>15:0</bits> <label>ScPosWidth</label> <comp a="1" b="4" c="0">nS</comp> </field>
//   <field> <bits>31:16</bits> <label>ScNegWidth</label> <comp a="1" b="4" c="0">nS</comp> </field>
//</register>
//<register> <name>TimeControl2</name> <address>18</address> <lane>0</lane> <vc>1</vc> <size>1</size>
//   <field> <bits>11:0</bits><label>ScCount</label></field>
//   <field> <bits>23:16</bits> <label>MckPosWidth</label> <comp a="1" b="4" c="0">nS</comp> </field>
//   <field> <bits>31:24</bits> <label>AdcClkPer</label> <comp a="1" b="4" c="0">nS</comp> </field>
//</register>
//<register> <name>TimeControl3</name> <address>19</address> <lane>0</lane> <vc>1</vc> <size>1</size>
//   <field> <bits>7:0</bits> <label>MckNegWidth</label> <comp a="1" b="4" c="0">nS</comp> </field>
//   <field> <bits>11:8</bits> <label>MckLimit</label> <comp a="0" b="1" c="1"/> </field>
//   <field> <bits>31:16</bits> <label>MckDelay</label> <comp a="1" b="4" c="0">nS</comp> </field>
//</register>
//<register> <name>TimeControl4</name> <address>20</address> <lane>0</lane> <vc>1</vc> <size>1</size>
//   <field> <bits>15:0</bits> <label>AdcDelay</label> <comp a="1" b="4" c="0">nS</comp> </field>
//  <field> <bits>31:16</bits> <label>AdcPhase</label> <comp a="1" b="4" c="0">nS</comp> </field>
//</register>
//<register> <name>TimeControl5</name> <address>21</address> <lane>0</lane> <vc>1</vc> <size>1</size>
//   <field> <bits>7:0</bits><label>PerMclkCount</label></field>
//   <field> <bits>19:8</bits> <label>SlowAdcDelay0</label> <comp a="1" b="4" c="0">nS</comp> </field>
//   <field> <bits>31:20</bits> <label>SlowAdcDelay1</label> <comp a="1" b="4" c="0">nS</comp> </field>
//</register>




class RegisterV1 {
  public:
    RegisterV1() {};
    ~RegisterV1() {};
    uint32_t addr;
    uint32_t shift;
    uint32_t mask;
    uint32_t defaultValue;
};

static uint32_t _regsfoo[ConfigV1::NumberOfRegisters][4] = {
    //addr shift  mask    default
            // GenControl
    { 0,    0,   0xf,        1},        //    AsicVersion
    { 0,    5,   0x1,        0},        //    CckDisable
    { 0,    6,   0x1,        0},        //    MckDisable
    { 0,    7,   0x1,        0},        //    EnExtTrig
    { 0,    8,   0xf,        0},        //    LemoSelect
    { 0,    12,  0x1,        1},        //    NoPayload
    { 0,    13,  0x1,        0},        //    ClkDisable
    { 0,    14,  0x1,        1},        //    AsicRstHwEn
            // TimeControl0
    { 1,    0,   0xff,       15},       //    PtDelay
    { 1,    8,   0xff,       255},      //    ScDelay
    { 1,    16,  0xff,       14},       //    CCkPosWidth
    { 1,    24,  0xff,       14},       //    CCkNegWidth
            // TimeControl1
    { 2,    0,   0xffff,     999},      //    ScPosWidth
    { 2,    16,  0xffff,     999},      //    ScNegWidth
            // TimeControl2
    { 3,    0,   0xfff,      15},       //    ScCount
    { 3,    16,  0xff,       19},       //    MckPosWidth
    { 3,    24,  0xff,       19},       //    AdcClkPer
            // TimeControl3
    { 4,    0,   0xff,       19},       //    MckNegWidth
    { 4,    8,   0xf,        0},        //    MckLimit
    { 4,    16,  0xffff,     174},      //    MckDelay
            // TimeControl4
    { 5,    0,   0xffff,     553},      //    AdcDelay
    { 5,    16,  0xffff,     35},       //    AdcPhase
            // TimeControl5
    { 6,    0,   0xff,       15},       //    PerMclkCount
    { 6,    8,   0xfff,      15},       //    SlowAdcDelay0
    { 6,    20,  0xfff,      365}       //    SlowAdcDelay1
};

static RegisterV1* _regs = (RegisterV1*) _regsfoo;

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

uint32_t   ConfigV1::get (Registers r) {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::get parameter out of range!! %u\n", r);
    return 0;
  }
  return ((_values[_regs[r].addr] >> _regs[r].shift) & _regs[r].mask);
}

const uint32_t ConfigV1::get (Registers r) const {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::get parameter out of range!! %u\n", r);
    return 0;
  }
  return ((_values[_regs[r].addr] >> _regs[r].shift) & _regs[r].mask);
}

uint32_t   ConfigV1::set (Registers r, uint32_t v) {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::set parameter out of range!! %u\n", r);
    return 0;
  }
  _values[_regs[r].addr] &= ~(_regs[r].mask << _regs[r].shift);
  _values[_regs[r].addr] |= (_regs[r].mask & v) << _regs[r].shift;
  return 0;
}

uint32_t   ConfigV1::rangeHigh(Registers r) {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::rangeHigh parameter out of range!! %u\n", r);
    return 0;
  }
  return _regs[r].mask;
}

uint32_t   ConfigV1::rangeLow(Registers r) {
  return 0;
}

uint32_t   ConfigV1::defaultValue(Registers r) {
  if (r >= ConfigV1::NumberOfRegisters) {
    printf("ConfigV1::defaultValue parameter out of range!! %u\n", r);
    return 0;
  }
  return _regs[r].defaultValue & _regs[r].mask;
 }

char*               ConfigV1::name     (Registers r, bool init) {
  static char _regsNames[NumberOfRegisters+1][120] = {
      {"AsicVersion"},    //      AsicVersion
      {"CckDisable"},     //      CckDisable
      {"MckDisable"},     //      MckDisable
      {"EnExtTrig"},      //      EnExtTrig
      {"LemoSelect"},     //      LemoSelect
      {"NoPayload"},      //      NoPayload
      {"ClkDisable"},     //      ClkDisable
      {"AsicRstHwEn"},    //      AsicRstHwEn
      {"PtDelay"},        //      PtDelay
      {"ScDelay"},        //      ScDelay
      {"CCkPosWidth"},    //      CCkPosWidth
      {"CCkNegWidth"},    //      CCkNegWidth
      {"ScPosWidth"},     //      ScPosWidth
      {"ScNegWidth"},     //      ScNegWidth
      {"ScCount"},        //      ScCount
      {"MckPosWidth"},    //      MckPosWidth
      {"AdcClkPer"},      //      AdcClkPer
      {"MckNegWidth"},    //      MckNegWidth
      {"MckLimit"},       //      MckLimit
      {"MckDelay"},       //      MckDelay
      {"AdcDelay"},       //      AdcDelay
      {"AdcPhase"},       //      AdcPhase
      {"PerMclkCount"},   //      PerMclkCount
      {"SlowAdcDelay0"},  //      SlowAdcDelay0
      {"SlowAdcDelay1"},  //      SlowAdcDelay1
    {"----INVALID----"}   //      NumberOfRegisters
  };
  static char range[60];

  if (init && (r < ConfigV1::NumberOfRegisters)) {
    sprintf(range, "  (%u..%u)    ", 0, _regs[r].mask);
    strncat(_regsNames[r], range, 40);
  }

  return r < ConfigV1::NumberOfRegisters ? _regsNames[r] : _regsNames[ConfigV1::NumberOfRegisters];

};

