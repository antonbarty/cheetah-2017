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
  printf("Voltage range: %hd\n", voltageRange());
  printf("First channel: %hd\n", firstChan());
  printf("Last channel: %hd\n", lastChan());
  printf("Input mode: %hd\n", inputMode());
  printf("Trigger mode: %hd\n", triggerMode());
  printf("Data format: %hd\n", dataFormat());
  printf("Frames per second: %hd\n", fps());
  printf("Autocalibrate enable: %s\n", autocalibEnable() ? "True" : "False");
  printf("Time tag enable: %s\n", timeTagEnable() ? "True" : "False");
  printf("------------------------------------\n");
}
