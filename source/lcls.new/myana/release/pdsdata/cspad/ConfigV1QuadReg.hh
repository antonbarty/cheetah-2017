#ifndef Cspad_ConfigV1QuadReg_hh
#define Cspad_ConfigV1QuadReg_hh

#include "Detector.hh"

#include <stdint.h>

#pragma pack(4)

namespace Pds {
  namespace CsPad {

    class CsPadDigitalPotsCfg
    {
      public:
        CsPadDigitalPotsCfg() {int i = 0; while (i<PotsPerQuad) pots[i++] = 0;}
        uint8_t         value(unsigned i) const {return pots[i];}
      public:
        uint8_t         pots[PotsPerQuad];
    };

    class CsPadReadOnlyCfg
    {
      public:
        CsPadReadOnlyCfg() : shiftTest(0), version(0) {};

      public:
        uint32_t        shiftTest;
        uint32_t        version;
    };

    class CsPadGainMapCfg
    {
      public:
        CsPadGainMapCfg() {};
        typedef uint16_t GainMap[ColumnsPerASIC][MaxRowsPerASIC];
        GainMap*       map()       { return &_gainMap; }
        const GainMap* map() const { return &_gainMap; }
      public:
        GainMap    _gainMap;
    };

    class ConfigV1QuadReg
    {
        class CsPadReadOnlyCfg;
        class CsPadDigitalPotsCfg;
        class CsPadGainMapCfg;

      public:
        ConfigV1QuadReg() {};
        ConfigV1QuadReg(
            uint32_t         shiftSelect[],
            uint32_t         edgeSelect[],
            uint32_t         readClkSet,
            uint32_t         readClkHold,
            uint32_t         dataMode,
            uint32_t         prstSel,
            uint32_t         acqDelay,
            uint32_t         intTime,
            uint32_t         digDelay,
            uint32_t         ampIdle,
            uint32_t         injTotal,
            uint32_t         rowColShiftPer) :
            _readClkSet(readClkSet),
            _readClkHold(readClkHold),
            _dataMode(dataMode),
            _prstSel(prstSel),
            _acqDelay(acqDelay),
            _intTime(intTime),
            _digDelay(digDelay),
            _ampIdle(ampIdle),
            _injTotal(injTotal),
            _rowColShiftPer(rowColShiftPer) {
          _shiftSelect[0] = shiftSelect[0];
          _shiftSelect[1] = shiftSelect[1];
          _shiftSelect[2] = shiftSelect[2];
          _shiftSelect[3] = shiftSelect[3];
          _edgeSelect [0] = edgeSelect [0];
          _edgeSelect [1] = edgeSelect [1];
          _edgeSelect [2] = edgeSelect [2];
          _edgeSelect [3] = edgeSelect [3];
        };

        const uint32_t*    shiftSelect()        const   { return _shiftSelect; }
        const uint32_t*    edgeSelect()         const   { return _edgeSelect;  }
        uint32_t           readClkSet()         const   { return _readClkSet;     }
        uint32_t           readClkHold()        const   { return _readClkHold;    }
        uint32_t           dataMode()           const   { return _dataMode;       }
        uint32_t           prstSel()            const   { return _prstSel;        }
        uint32_t           acqDelay()           const   { return _acqDelay;       }
        uint32_t           intTime()            const   { return _intTime;        }
        uint32_t           digDelay()           const   { return _digDelay;       }
        uint32_t           ampIdle()            const   { return _ampIdle;        }
        uint32_t           injTotal()           const   { return _injTotal;       }
        uint32_t           rowColShiftPer()     const   { return _rowColShiftPer; }
        Pds::CsPad::CsPadReadOnlyCfg&           ro      ()        { return _readOnly;       }
        const Pds::CsPad::CsPadReadOnlyCfg&     ro      ()  const { return _readOnly;       }
        Pds::CsPad::CsPadDigitalPotsCfg&        dp      ()        { return _digitalPots;    }
        const Pds::CsPad::CsPadDigitalPotsCfg&  dp      ()  const { return _digitalPots;    }
        Pds::CsPad::CsPadGainMapCfg*            gm      ()        { return &_gainMap;       }
        const Pds::CsPad::CsPadGainMapCfg*      gm      ()  const { return &_gainMap;       }
        Pds::CsPad::CsPadReadOnlyCfg*           readOnly()        { return &_readOnly;      }
        const Pds::CsPad::CsPadReadOnlyCfg*     readOnly()  const { return &_readOnly;      }

      private:
        uint32_t                     _shiftSelect[TwoByTwosPerQuad];
        uint32_t                     _edgeSelect[TwoByTwosPerQuad];
        uint32_t                     _readClkSet;
        uint32_t                     _readClkHold;
        uint32_t                     _dataMode;
        uint32_t                     _prstSel;
        uint32_t                     _acqDelay;
        uint32_t                     _intTime;
        uint32_t                     _digDelay;
        uint32_t                     _ampIdle;
        uint32_t                     _injTotal;
        uint32_t                     _rowColShiftPer;

        Pds::CsPad::CsPadReadOnlyCfg     _readOnly;
        Pds::CsPad::CsPadDigitalPotsCfg  _digitalPots;
        Pds::CsPad::CsPadGainMapCfg      _gainMap;
    };
  };
};
#pragma pack()

#endif
