#include "pdsdata/timepix/ConfigV2.hh"
#include <stdio.h>
#include <string.h>

using namespace Pds;
using namespace Timepix;

ConfigV2::ConfigV2 ()
{}

ConfigV2::ConfigV2 (
  uint8_t   readoutSpeed,
  uint8_t   triggerMode,
  int32_t   timepixSpeed,
  int32_t   dac0Ikrum,
  int32_t   dac0Disc,
  int32_t   dac0Preamp,
  int32_t   dac0BufAnalogA,
  int32_t   dac0BufAnalogB,
  int32_t   dac0Hist,
  int32_t   dac0ThlFine,
  int32_t   dac0ThlCourse,
  int32_t   dac0Vcas,
  int32_t   dac0Fbk,
  int32_t   dac0Gnd,
  int32_t   dac0Ths,
  int32_t   dac0BiasLvds,
  int32_t   dac0RefLvds,
  int32_t   dac1Ikrum,
  int32_t   dac1Disc,
  int32_t   dac1Preamp,
  int32_t   dac1BufAnalogA,
  int32_t   dac1BufAnalogB,
  int32_t   dac1Hist,
  int32_t   dac1ThlFine,
  int32_t   dac1ThlCourse,
  int32_t   dac1Vcas,
  int32_t   dac1Fbk,
  int32_t   dac1Gnd,
  int32_t   dac1Ths,
  int32_t   dac1BiasLvds,
  int32_t   dac1RefLvds,
  int32_t   dac2Ikrum,
  int32_t   dac2Disc,
  int32_t   dac2Preamp,
  int32_t   dac2BufAnalogA,
  int32_t   dac2BufAnalogB,
  int32_t   dac2Hist,
  int32_t   dac2ThlFine,
  int32_t   dac2ThlCourse,
  int32_t   dac2Vcas,
  int32_t   dac2Fbk,
  int32_t   dac2Gnd,
  int32_t   dac2Ths,
  int32_t   dac2BiasLvds,
  int32_t   dac2RefLvds,
  int32_t   dac3Ikrum,
  int32_t   dac3Disc,
  int32_t   dac3Preamp,
  int32_t   dac3BufAnalogA,
  int32_t   dac3BufAnalogB,
  int32_t   dac3Hist,
  int32_t   dac3ThlFine,
  int32_t   dac3ThlCourse,
  int32_t   dac3Vcas,
  int32_t   dac3Fbk,
  int32_t   dac3Gnd,
  int32_t   dac3Ths,
  int32_t   dac3BiasLvds,
  int32_t   dac3RefLvds
  ) :
  _readoutSpeed(readoutSpeed),
  _triggerMode(triggerMode),
  _timepixSpeed(timepixSpeed),
  _dac0Ikrum(dac0Ikrum),
  _dac0Disc(dac0Disc),
  _dac0Preamp(dac0Preamp),
  _dac0BufAnalogA(dac0BufAnalogA),
  _dac0BufAnalogB(dac0BufAnalogB),
  _dac0Hist(dac0Hist),
  _dac0ThlFine(dac0ThlFine),
  _dac0ThlCourse(dac0ThlCourse),
  _dac0Vcas(dac0Vcas),
  _dac0Fbk(dac0Fbk),
  _dac0Gnd(dac0Gnd),
  _dac0Ths(dac0Ths),
  _dac0BiasLvds(dac0BiasLvds),
  _dac0RefLvds(dac0RefLvds),
  _dac1Ikrum(dac1Ikrum),
  _dac1Disc(dac1Disc),
  _dac1Preamp(dac1Preamp),
  _dac1BufAnalogA(dac1BufAnalogA),
  _dac1BufAnalogB(dac1BufAnalogB),
  _dac1Hist(dac1Hist),
  _dac1ThlFine(dac1ThlFine),
  _dac1ThlCourse(dac1ThlCourse),
  _dac1Vcas(dac1Vcas),
  _dac1Fbk(dac1Fbk),
  _dac1Gnd(dac1Gnd),
  _dac1Ths(dac1Ths),
  _dac1BiasLvds(dac1BiasLvds),
  _dac1RefLvds(dac1RefLvds),
  _dac2Ikrum(dac2Ikrum),
  _dac2Disc(dac2Disc),
  _dac2Preamp(dac2Preamp),
  _dac2BufAnalogA(dac2BufAnalogA),
  _dac2BufAnalogB(dac2BufAnalogB),
  _dac2Hist(dac2Hist),
  _dac2ThlFine(dac2ThlFine),
  _dac2ThlCourse(dac2ThlCourse),
  _dac2Vcas(dac2Vcas),
  _dac2Fbk(dac2Fbk),
  _dac2Gnd(dac2Gnd),
  _dac2Ths(dac2Ths),
  _dac2BiasLvds(dac2BiasLvds),
  _dac2RefLvds(dac2RefLvds),
  _dac3Ikrum(dac3Ikrum),
  _dac3Disc(dac3Disc),
  _dac3Preamp(dac3Preamp),
  _dac3BufAnalogA(dac3BufAnalogA),
  _dac3BufAnalogB(dac3BufAnalogB),
  _dac3Hist(dac3Hist),
  _dac3ThlFine(dac3ThlFine),
  _dac3ThlCourse(dac3ThlCourse),
  _dac3Vcas(dac3Vcas),
  _dac3Fbk(dac3Fbk),
  _dac3Gnd(dac3Gnd),
  _dac3Ths(dac3Ths),
  _dac3BiasLvds(dac3BiasLvds),
  _dac3RefLvds(dac3RefLvds)
{
  _driverVersion = _firmwareVersion = 0;
  _pixelThreshSize = 0;
  memset(_pixelThresh, 0, sizeof(_pixelThresh));
  memset(_chip0Name, 0, sizeof(_chip0Name));
  memset(_chip1Name, 0, sizeof(_chip1Name));
  memset(_chip2Name, 0, sizeof(_chip2Name));
  memset(_chip3Name, 0, sizeof(_chip3Name));
  _chip0ID = _chip1ID = _chip2ID = _chip3ID = 0;
}

const char *ConfigV2::chip0Name() const
{
  return (_chip0Name);
}

void ConfigV2::chip0Name(const char *name)
{
  strncpy(_chip0Name, name, ChipNameMax);
}

const char *ConfigV2::chip1Name() const
{
  return (_chip1Name);
}

void ConfigV2::chip1Name(const char *name)
{
  strncpy(_chip1Name, name, ChipNameMax);
}

const char *ConfigV2::chip2Name() const
{
  return (_chip2Name);
}

void ConfigV2::chip2Name(const char *name)
{
  strncpy(_chip2Name, name, ChipNameMax);
}

const char *ConfigV2::chip3Name() const
{
  return (_chip3Name);
}

void ConfigV2::chip3Name(const char *name)
{
  strncpy(_chip3Name, name, ChipNameMax);
}

void ConfigV2::driverVersion(int32_t version)
{
  _driverVersion = version;
}

int32_t ConfigV2::driverVersion() const
{
  return (_driverVersion);
}

void ConfigV2::firmwareVersion(uint32_t version)
{
  _firmwareVersion = version;
}

uint32_t ConfigV2::firmwareVersion() const
{
  return (_firmwareVersion);
}

void ConfigV2::pixelThresh(uint32_t threshSize, uint8_t *thresh)
{
  _pixelThreshSize = threshSize;
  if ((_pixelThreshSize > 0) && (_pixelThreshSize <= PixelThreshMax)) {
    memcpy(_pixelThresh, thresh, _pixelThreshSize);
  }
}

uint32_t ConfigV2::pixelThreshSize() const
{
  return (_pixelThreshSize);
}

const uint8_t * ConfigV2::pixelThresh() const
{
  return (_pixelThresh);
}

void ConfigV2::dump() const {
  printf("-------Timepix Config---------------\n");
  printf("Readout speed: ");
  switch (readoutSpeed()) {
    case ReadoutSpeed_Slow:
      printf("62.5 MHz");
      break;
    case ReadoutSpeed_Fast:
      printf("125 MHz");
      break;
    default:
      printf("Invalid");
      break;
  }
  printf("\n");

  printf("Timepix speed: ");
  switch (timepixSpeed()) {
    case 0:
      printf("100 MHz");
      break;
    case 1:
      printf("80 MHz");
      break;
    case 2:
      printf("40 MHz");
      break;
    case 3:
      printf("10 MHz");
      break;
    case 4:
      printf("2.5 MHz");
      break;
    default:
      printf("Invalid");
      break;
  }
  printf("\n");

  printf("DAC0 thl fine: %d\n", dac0ThlFine());
  printf("DAC1 thl fine: %d\n", dac1ThlFine());
  printf("DAC2 thl fine: %d\n", dac2ThlFine());
  printf("DAC3 thl fine: %d\n", dac3ThlFine());

  printf("DAC0 ikrum: %d\n", dac0Ikrum());
  printf("DAC0 disc: %d\n", dac0Disc());
  printf("DAC0 preamp: %d\n", dac0Preamp());
  printf("DAC0 buf analog A: %d\n", dac0BufAnalogA());
  printf("DAC0 buf analog B: %d\n", dac0BufAnalogB());
  printf("DAC0 hist: %d\n", dac0Hist());
  printf("DAC0 thl course: %d\n", dac0ThlCourse());
  printf("DAC0 vcas: %d\n", dac0Vcas());
  printf("DAC0 fbk: %d\n", dac0Fbk());
  printf("DAC0 gnd: %d\n", dac0Gnd());
  printf("DAC0 ths: %d\n", dac0Ths());
  printf("DAC0 bias lvds: %d\n", dac0BiasLvds());
  printf("DAC0 ref lvds: %d\n", dac0RefLvds());

  printf("DAC1 ikrum: %d\n", dac1Ikrum());
  printf("DAC1 disc: %d\n", dac1Disc());
  printf("DAC1 preamp: %d\n", dac1Preamp());
  printf("DAC1 buf analog A: %d\n", dac1BufAnalogA());
  printf("DAC1 buf analog B: %d\n", dac1BufAnalogB());
  printf("DAC1 hist: %d\n", dac1Hist());
  printf("DAC1 thl course: %d\n", dac1ThlCourse());
  printf("DAC1 vcas: %d\n", dac1Vcas());
  printf("DAC1 fbk: %d\n", dac1Fbk());
  printf("DAC1 gnd: %d\n", dac1Gnd());
  printf("DAC1 ths: %d\n", dac1Ths());
  printf("DAC1 bias lvds: %d\n", dac1BiasLvds());
  printf("DAC1 ref lvds: %d\n", dac1RefLvds());

  printf("DAC2 ikrum: %d\n", dac2Ikrum());
  printf("DAC2 disc: %d\n", dac2Disc());
  printf("DAC2 preamp: %d\n", dac2Preamp());
  printf("DAC2 buf analog A: %d\n", dac2BufAnalogA());
  printf("DAC2 buf analog B: %d\n", dac2BufAnalogB());
  printf("DAC2 hist: %d\n", dac2Hist());
  printf("DAC2 thl course: %d\n", dac2ThlCourse());
  printf("DAC2 vcas: %d\n", dac2Vcas());
  printf("DAC2 fbk: %d\n", dac2Fbk());
  printf("DAC2 gnd: %d\n", dac2Gnd());
  printf("DAC2 ths: %d\n", dac2Ths());
  printf("DAC2 bias lvds: %d\n", dac2BiasLvds());
  printf("DAC2 ref lvds: %d\n", dac2RefLvds());

  printf("DAC3 ikrum: %d\n", dac3Ikrum());
  printf("DAC3 disc: %d\n", dac3Disc());
  printf("DAC3 preamp: %d\n", dac3Preamp());
  printf("DAC3 buf analog A: %d\n", dac3BufAnalogA());
  printf("DAC3 buf analog B: %d\n", dac3BufAnalogB());
  printf("DAC3 hist: %d\n", dac3Hist());
  printf("DAC3 thl course: %d\n", dac3ThlCourse());
  printf("DAC3 vcas: %d\n", dac3Vcas());
  printf("DAC3 fbk: %d\n", dac3Fbk());
  printf("DAC3 gnd: %d\n", dac3Gnd());
  printf("DAC3 ths: %d\n", dac3Ths());
  printf("DAC3 bias lvds: %d\n", dac3BiasLvds());
  printf("DAC3 ref lvds: %d\n", dac3RefLvds());

  printf("Trigger mode: ");
  switch (triggerMode()) {
    case TriggerMode_ExtPos:
      printf("External/Positive");
      break;
    case TriggerMode_ExtNeg:
      printf("External/Negative");
      break;
    case TriggerMode_Soft:
      printf("Software");
      break;
    default:
      printf("Invalid");
      break;
  }
  printf("\n");

  printf("Driver version: %d.%d.%d\n", driverVersion() / 100,
                                       (driverVersion() / 10) % 10,
                                       (driverVersion() % 10));

  printf("Firmware version: %d.%d.%d\n", firmwareVersion() / 100,
                                       (firmwareVersion() / 10) % 10,
                                       (firmwareVersion() % 10));

  printf("Chip 0 name: %-15s ID: 0x%08x\n", chip0Name(), chip0ID());
  printf("Chip 1 name: %-15s ID: 0x%08x\n", chip1Name(), chip1ID());
  printf("Chip 2 name: %-15s ID: 0x%08x\n", chip2Name(), chip2ID());
  printf("Chip 3 name: %-15s ID: 0x%08x\n", chip3Name(), chip3ID());

  printf("------------------------------------\n");
}
