#include "pdsdata/gsc16ai/ConfigV1.hh"
#include <stdio.h>

using namespace Pds;
using namespace Gsc16ai;

ConfigV1::ConfigV1 () {}

ConfigV1::ConfigV1 (
  uint16_t    voltageRange,
  uint16_t    firstChan,
  uint16_t    lastChan,
  bool        autocalibEnable,
  uint16_t    inputMode,
  uint16_t    triggerMode,
  uint16_t    dataFormat,
  uint16_t    fps,
  bool        timeTagEnable
  ) :
  _voltageRange (voltageRange),
  _firstChan    (firstChan),
  _lastChan     (lastChan),
  _inputMode    (inputMode),
  _triggerMode  (triggerMode),
  _dataFormat   (dataFormat),
  _fps          (fps),
  _autocalibEnable(autocalibEnable ? 1 : 0),
  _timeTagEnable(timeTagEnable ? 1 : 0)
{}

void ConfigV1::dump() const {
  printf("-------Gsc16ai Config---------------\n");
  printf("Voltage range: ");
  switch (voltageRange()) {
    case VoltageRange_10V:  printf("+/- 10V");  break;
    case VoltageRange_5V:   printf("+/- 5V");   break;
    case VoltageRange_2_5V: printf("+/- 2.5V"); break;
    default:                printf("Invalid");  break;
  }
  printf("\nFirst channel: %hd\n", firstChan());
  printf("Last channel: %hd\n", lastChan());
  printf("Input mode: ");
  switch (inputMode()) {
    case InputMode_Differential:  printf("Differential"); break;
    case InputMode_Zero:          printf("Zero");         break;
    case InputMode_Vref:          printf("Vref");         break;
    default:                      printf("Invalid");      break;
  }
  printf("\nTrigger mode: ");
  switch (triggerMode()) {
    case TriggerMode_ExtPos: printf("External/POS");                break;
    case TriggerMode_ExtNeg: printf("External/NEG");                break;
    case TriggerMode_IntClk: printf("Internal Clk, %hd Hz", fps()); break;
    default:                 printf("Invalid");                     break;
  }
  printf("\nData format: ");
  switch (dataFormat()) {
    case DataFormat_TwosComplement: printf("Two's Complement"); break;
    case DataFormat_OffsetBinary:   printf("Offset Binary");    break;
    default:                        printf("Invalid");          break;
  }
  printf("\nAutocalibrate enable: %s\n", autocalibEnable() ? "True" : "False");
  printf("Time tag enable: %s\n", timeTagEnable() ? "True" : "False");
  printf("------------------------------------\n");
}
