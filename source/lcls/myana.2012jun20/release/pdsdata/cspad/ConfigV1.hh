#ifndef CSPAD_CONFIG_V1_HH
#define CSPAD_CONFIG_V1_HH

#include "Detector.hh"
#include "ConfigV1QuadReg.hh"

#include <stdint.h>

#pragma pack(4)

namespace Pds
{
  namespace CsPad
  {
    class ConfigV1
    {
      public:
        static const int Version               = 1;
        ConfigV1() {};
        ConfigV1(
            uint32_t runDelay,
            uint32_t eventCode,
            uint32_t inactiveRunMode,
            uint32_t activeRunMode,
            uint32_t testDataIndex,
            uint32_t payloadPerQuad,
            uint32_t badAsicMask0,
            uint32_t badAsicMask1,
            uint32_t AsicMask,
            uint32_t quadMask) :
              _concentratorVersion(0),
              _runDelay(runDelay),
              _eventCode(eventCode),
              _inactiveRunMode(inactiveRunMode),
              _activeRunMode(activeRunMode),
              _testDataIndex(testDataIndex),
              _payloadPerQuad(payloadPerQuad),
              _badAsicMask0(badAsicMask0),
              _badAsicMask1(badAsicMask1),
              _AsicMask(AsicMask),
              _quadMask(quadMask) {};

        ConfigV1QuadReg*       quads        ()       { return _quads; }
        const ConfigV1QuadReg* quads        () const { return _quads; }
        uint32_t               tdi          () const { return _testDataIndex; }
        uint32_t               quadMask     () const { return _quadMask; }
        uint32_t               runDelay     () const { return _runDelay; }
        uint32_t               eventCode    () const { return _eventCode; }
        uint32_t               inactiveRunMode()const{ return _inactiveRunMode; }
        uint32_t               activeRunMode() const { return _activeRunMode; }
        uint32_t               payloadSize  () const { return _payloadPerQuad; }
        uint32_t               badAsicMask0 () const { return _badAsicMask0; }
        uint32_t               badAsicMask1 () const { return _badAsicMask1; }
        uint32_t               asicMask     () const { return _AsicMask; }
        uint32_t               numAsicsRead () const { return (_AsicMask&0xf)==1 ? 4 : 16; }
        uint32_t               concentratorVersion() const {return _concentratorVersion; }
        uint32_t*              concentratorVersionAddr() {return &_concentratorVersion; }
        static const int       version      ()       { return Version; }
      private:
        uint32_t          _concentratorVersion;
        uint32_t          _runDelay;
        uint32_t          _eventCode;
        uint32_t          _inactiveRunMode;
        uint32_t          _activeRunMode;
        uint32_t          _testDataIndex;
        uint32_t          _payloadPerQuad;
        uint32_t          _badAsicMask0;
        uint32_t          _badAsicMask1;
        uint32_t          _AsicMask;  // there are a maximum of 64 ASICs
        uint32_t          _quadMask;
        ConfigV1QuadReg   _quads[MaxQuadsPerSensor];
    };


  } // namespace CsPad

} // namespace Pds 

#pragma pack()

#endif  // CSPAD_CONFIG_V1_HH
