/*
 * ChannelV1.cc
 *
 *  Created on: May 31, 2011
 *      Author: jackp
 */

#include "pdsdata/xamps/ChannelV1.hh"
#include <stdio.h>
#include <string.h>

using namespace Pds::Xamps;

class BitFieldV1 {
  public:
    BitFieldV1() {};
    ~BitFieldV1() {};
  public:
    uint32_t shift;
    uint32_t mask;
    uint32_t defaultValue;
};

static uint32_t _chfoo[][3] = {
   //shift mask
    {  0,   7,  0 },    //    TrimBits,
    {  4,   1,  0 },    //    EnableTest,
    {  8,   1,  0 },    //    ChannelMask,
    { 12,   1,  0 }    //    ChannelSelectorEnable
};

static BitFieldV1* _chs = (BitFieldV1*) _chfoo;

static bool           namesAreInitialized = false;

ChannelV1::ChannelV1() {
  if (!namesAreInitialized){
    int e;
    for (e=0; e< (int)ChannelV1::NumberOfChannelBitFields; e++) {
      name((ChannelV1::ChannelBitFields)e, true);
    }
    namesAreInitialized = true;
  }
}

uint32_t        ChannelV1::get      (ChannelBitFields e) {
  if (e >= ChannelV1::NumberOfChannelBitFields) {
    printf("ChannelV1:: parameter out of range!! %u\n", e);
    return 0;
  }
  return ((_value >> _chs[e].shift) & _chs[e].mask);
}

const uint32_t  ChannelV1::get      (ChannelBitFields e) const {
  if (e >= ChannelV1::NumberOfChannelBitFields) {
    printf("ChannelV1:: parameter out of range!! %u\n", e);
    return 0;
  }
  return ((_value >> _chs[e].shift) & _chs[e].mask);
}

uint32_t        ChannelV1::set      (ChannelBitFields e, uint32_t v) {
  if (e >= ChannelV1::NumberOfChannelBitFields) {
    printf("ChannelV1:: parameter out of range!! %u\n", e);
    return 0;
  }
  _value &= ~(_chs[e].mask << _chs[e].shift);
  _value |= (_chs[e].mask & v) << _chs[e].shift;
  return 0;
}

uint32_t        ChannelV1::rangeHigh(ChannelBitFields e) {
  if (e >= ChannelV1::NumberOfChannelBitFields) {
    printf("ChannelV1:: parameter out of range!! %u\n", e);
    return 0;
  }
  return _chs[e].mask;
}

uint32_t        ChannelV1::rangeLow (ChannelBitFields e) {
  return 0;
}

uint32_t            ChannelV1::defaultValue(ChannelBitFields e) {
  if (e >= ChannelV1::NumberOfChannelBitFields) {
    printf("ChannelV1::defaultValue parameter of of range!! %u\n", e);
    return 0;
  }
  return _chs[e].defaultValue & _chs[e].mask;
}

char*      ChannelV1::name     (ChannelBitFields e, bool init) {
  static char _chNames[ChannelV1::NumberOfChannelBitFields + 1][120] = {
      {"TrimBits             "},              //    TrimBits,
      {"EnableTest           "},            //    EnableTest,
      {"ChannelMask          "},           //    ChannelMask,
      {"ChannelSelectorEnable"}, //    ChannelSelectorEnable
      {"------INVALID--------"}            //    NumberOfChannelBitFields
  };
  static char range[60];

  if (init && (e < ChannelV1::NumberOfChannelBitFields)) {
    sprintf(range, "  (%u..%u)    ", 0, _chs[e].mask);
    strncat(_chNames[e], range, 40);
  }

  return e < ChannelV1::NumberOfChannelBitFields ? _chNames[e] : _chNames[ChannelV1::NumberOfChannelBitFields];
}
