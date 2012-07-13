#ifndef Cspad2x2_ConfigV1QuadReg_hh
#define Cspad2x2_ConfigV1QuadReg_hh

#include "pdsdata/cspad2x2/Detector.hh"

#include <stdint.h>

#pragma pack(4)

namespace Pds {
  namespace CsPad2x2 {

    class CsPad2x2DigitalPotsCfg
    {
      public:
        CsPad2x2DigitalPotsCfg() {int i = 0; while (i<PotsPerQuad) pots[i++] = 0;}
        uint8_t         value(unsigned i) const {return pots[i];}
      public:
        uint8_t         pots[PotsPerQuad];
    };

    class CsPad2x2ReadOnlyCfg
    {
      public:
        CsPad2x2ReadOnlyCfg() : shiftTest(0), version(0) {};
        ~CsPad2x2ReadOnlyCfg() {};

      public:
        uint32_t        shiftTest;
        uint32_t        version;
    };

    class CsPad2x2GainMapCfg
    {
      public:
        CsPad2x2GainMapCfg() {};
        typedef uint16_t GainMap[ColumnsPerASIC][MaxRowsPerASIC];
        GainMap*       map()       { return &_gainMap; }
        const GainMap* map() const { return &_gainMap; }
      public:
        GainMap    _gainMap;
    };

    class ConfigV1QuadReg
    {
        class CsPad2x2ReadOnlyCfg;
        class CsPad2x2DigitalPotsCfg;
        class CsPad2x2GainMapCfg;

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
            uint32_t         rowColShiftPer,
            uint32_t         ampReset,
            uint32_t         digCount,
            uint32_t         digPeriod,
            uint32_t         PeltierEnable,
            uint32_t         kpConstant,
            uint32_t         kiConstant,
            uint32_t         kdConstant,
            uint32_t         humidThold,
            uint32_t         setPoint  ) :
            _readClkSet(readClkSet),
            _readClkHold(readClkHold),
            _dataMode(dataMode),
            _prstSel(prstSel),
            _acqDelay(acqDelay),
            _intTime(intTime),
            _digDelay(digDelay),
            _ampIdle(ampIdle),
            _injTotal(injTotal),
            _rowColShiftPer(rowColShiftPer),
            _ampReset(ampReset),
            _digCount(digCount),
            _digPeriod(digPeriod),
            _PeltierEnable(PeltierEnable),
            _kpConstant(kpConstant),
            _kiConstant(kiConstant),
            _kdConstant(kdConstant),
            _humidThold(humidThold),
            _setPoint(setPoint)
        {
          for (unsigned i=0; i<TwoByTwosPerQuad; i++) _shiftSelect[i] = shiftSelect[i];
          for (unsigned i=0; i<TwoByTwosPerQuad; i++) _edgeSelect [i] = edgeSelect [i];
        };

        const uint32_t*                            shiftSelect()        const { return _shiftSelect;    }
        const uint32_t*                            edgeSelect()         const { return _edgeSelect;     }
        uint32_t                                   readClkSet()         const { return _readClkSet;     }
        uint32_t                                   readClkHold()        const { return _readClkHold;    }
        uint32_t                                   dataMode()           const { return _dataMode;       }
        uint32_t                                   prstSel()            const { return _prstSel;        }
        uint32_t                                   acqDelay()           const { return _acqDelay;       }
        uint32_t                                   intTime()            const { return _intTime;        }
        uint32_t                                   digDelay()           const { return _digDelay;       }
        uint32_t                                   ampIdle()            const { return _ampIdle;        }
        uint32_t                                   injTotal()           const { return _injTotal;       }
        uint32_t                                   rowColShiftPer()     const { return _rowColShiftPer; }
        uint32_t                                   ampReset()           const { return _ampReset;       }
        uint32_t                                   digCount()           const { return _digCount;       }
        uint32_t                                   digPeriod()          const { return _digPeriod;      }
        uint32_t                                   PeltierEnable()      const { return _PeltierEnable;  }
        uint32_t                                   kpConstant()         const { return _kpConstant;     }
        uint32_t                                   kiConstant()         const { return _kiConstant;     }
        uint32_t                                   kdConstant()         const { return _kdConstant;     }
        uint32_t                                   humidThold()         const { return _humidThold;     }
        uint32_t                                   setPoint()           const { return _setPoint;       }
        Pds::CsPad2x2::CsPad2x2ReadOnlyCfg&                 ro()              { return _readOnly;       }
        const Pds::CsPad2x2::CsPad2x2ReadOnlyCfg&           ro()        const { return _readOnly;       }
        Pds::CsPad2x2::CsPad2x2DigitalPotsCfg&              dp()              { return _digitalPots;    }
        const Pds::CsPad2x2::CsPad2x2DigitalPotsCfg&        dp()        const { return _digitalPots;    }
        Pds::CsPad2x2::CsPad2x2GainMapCfg*                  gm()              { return &_gainMap;       }
        const Pds::CsPad2x2::CsPad2x2GainMapCfg*            gm()        const { return &_gainMap;       }
        Pds::CsPad2x2::CsPad2x2ReadOnlyCfg*                 readOnly()        { return &_readOnly;      }
        const Pds::CsPad2x2::CsPad2x2ReadOnlyCfg*           readOnly()  const { return &_readOnly;      }

      private:
        uint32_t                            _shiftSelect[TwoByTwosPerQuad];
        uint32_t                            _edgeSelect[TwoByTwosPerQuad];
        uint32_t                            _readClkSet;
        uint32_t                            _readClkHold;
        uint32_t                            _dataMode;
        uint32_t                            _prstSel;
        uint32_t                            _acqDelay;
        uint32_t                            _intTime;
        uint32_t                            _digDelay;
        uint32_t                            _ampIdle;
        uint32_t                            _injTotal;
        uint32_t                            _rowColShiftPer;
        uint32_t                            _ampReset;
        uint32_t                            _digCount;
        uint32_t                            _digPeriod;
        uint32_t                            _PeltierEnable;
        uint32_t                            _kpConstant;
        uint32_t                            _kiConstant;
        uint32_t                            _kdConstant;
        uint32_t                            _humidThold;
        uint32_t                            _setPoint;

        Pds::CsPad2x2::CsPad2x2ReadOnlyCfg     _readOnly;
        Pds::CsPad2x2::CsPad2x2DigitalPotsCfg  _digitalPots;
        Pds::CsPad2x2::CsPad2x2GainMapCfg      _gainMap;
    };
  };
};

#pragma pack()

#endif
