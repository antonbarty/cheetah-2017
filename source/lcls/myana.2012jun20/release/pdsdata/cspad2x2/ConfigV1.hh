#ifndef Cspad2x2_ConfigV1_hh
#define Cspad2x2_ConfigV1_hh

//
//  CSPAD2x2 readout configuration and region of interest reduction
//

#include "pdsdata/cspad2x2/Detector.hh"
#include "pdsdata/cspad2x2/ConfigV1QuadReg.hh"

#include <stdint.h>

#pragma pack(4)

namespace Pds
{
  namespace CsPad2x2
  {
    class ProtectionSystemThreshold
    {
      public:
        ProtectionSystemThreshold() {};
        ~ProtectionSystemThreshold() {};

      public:
        uint32_t adcThreshold;
        uint32_t pixelCountThreshold;
    };

    class ConfigV1
    {
      public:
        static const int Version               = 1;
        ConfigV1() {};
        ConfigV1(
            uint32_t inactiveRunMode,
            uint32_t activeRunMode,
            uint32_t testDataIndex,
            uint32_t payloadPerQuad,
            uint32_t badAsicMask0,
            uint32_t AsicMask,
	        uint32_t roiMask);

        ConfigV1QuadReg*                 quad();
        const ConfigV1QuadReg*           quad() const;
        uint32_t                         tdi() const { return _testDataIndex; }
        ProtectionSystemThreshold*       protectionThreshold ();
        const ProtectionSystemThreshold* protectionThreshold () const;
        const uint32_t                   protectionEnable () const { return _protectionEnable; }
        uint32_t                         protectionEnable ()       { return _protectionEnable; }
        void                             protectionEnable(uint32_t e) {_protectionEnable=e; }
        uint32_t                         inactiveRunMode()const{ return _inactiveRunMode; }
        uint32_t                         activeRunMode() const { return _activeRunMode; }
        uint32_t                         payloadSize  () const { return _payloadPerQuad; }
        uint32_t                         badAsicMask () const { return _badAsicMask; }
        //  "asicMask" is actually a 4-bit mask of 2x2 sectors applied to all quadrants
        uint32_t                         asicMask     () const { return _AsicMask; }
        //  "roiMask" is a mask of 2x1 slices
        unsigned                         roiMask      (int iq) const;
        unsigned                         roiMask      () const;
        unsigned                         numAsicsRead () const;
        unsigned                         numAsicsStored(int iq) const;
        unsigned                         numAsicsStored() const;
        uint32_t                         concentratorVersion() const {return _concentratorVersion; }
        uint32_t*                        concentratorVersionAddr();
        static const int                 version      ()       { return Version; }
      private:
        uint32_t                  _concentratorVersion;
        ProtectionSystemThreshold _protectionThreshold;
        uint32_t                  _protectionEnable;
        uint32_t                  _inactiveRunMode;
        uint32_t                  _activeRunMode;
        uint32_t                  _testDataIndex;
        uint32_t                  _payloadPerQuad;
        uint32_t                  _badAsicMask;
        uint32_t                  _AsicMask;  // there are a maximum of 4 ASICs
        uint32_t                  _roiMask;
        ConfigV1QuadReg           _quad;
    };


  } // namespace CsPad2x2

} // namespace Pds 

#pragma pack()

#endif  // CSPAD_CONFIG_V1_HH
