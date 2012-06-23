// $Id: ConfigV2.hh,v 1.2 2012/03/14 00:49:39 caf Exp $
// Author: Chris Ford <caf@slac.stanford.edu>

//
//  Class for configuration of Timepix
//
#ifndef TIMEPIXCONFIGV2_HH
#define TIMEPIXCONFIGV2_HH

#include <stdint.h>

#pragma pack(4)
namespace Pds {

  namespace Timepix {

    class ConfigV2 {
    public:
      enum { Version=2 };
      enum ReadoutSpeed { ReadoutSpeed_Slow=0, ReadoutSpeed_Fast };
      enum TriggerMode  { TriggerMode_ExtPos=0, TriggerMode_ExtNeg, TriggerMode_Soft };
      enum { ChipCount=4 };
      enum { ChipNameMax=15 };
      enum { PixelThreshMax=ChipCount*256*256 };

      ConfigV2();

      ConfigV2(uint8_t readoutSpeed, uint8_t triggerMode, int32_t timepixSpeed,
               int32_t dac0Ikrum, int32_t dac0Disc, int32_t dac0Preamp,
               int32_t dac0BufAnalogA, int32_t dac0BufAnalogB, int32_t dac0Hist,
               int32_t dac0ThlFine, int32_t dac0ThlCourse, int32_t dac0Vcas,
               int32_t dac0Fbk, int32_t dac0Gnd, int32_t dac0Ths,
               int32_t dac0BiasLvds, int32_t dac0RefLvds,
               int32_t dac1Ikrum, int32_t dac1Disc, int32_t dac1Preamp,
               int32_t dac1BufAnalogA, int32_t dac1BufAnalogB, int32_t dac1Hist,
               int32_t dac1ThlFine, int32_t dac1ThlCourse, int32_t dac1Vcas,
               int32_t dac1Fbk, int32_t dac1Gnd, int32_t dac1Ths,
               int32_t dac1BiasLvds, int32_t dac1RefLvds,
               int32_t dac2Ikrum, int32_t dac2Disc, int32_t dac2Preamp,
               int32_t dac2BufAnalogA, int32_t dac2BufAnalogB, int32_t dac2Hist,
               int32_t dac2ThlFine, int32_t dac2ThlCourse, int32_t dac2Vcas,
               int32_t dac2Fbk, int32_t dac2Gnd, int32_t dac2Ths,
               int32_t dac2BiasLvds, int32_t dac2RefLvds,
               int32_t dac3Ikrum, int32_t dac3Disc, int32_t dac3Preamp,
               int32_t dac3BufAnalogA, int32_t dac3BufAnalogB, int32_t dac3Hist,
               int32_t dac3ThlFine, int32_t dac3ThlCourse, int32_t dac3Vcas,
               int32_t dac3Fbk, int32_t dac3Gnd, int32_t dac3Ths,
               int32_t dac3BiasLvds, int32_t dac3RefLvds);

      unsigned          size()              const     { return (unsigned) sizeof(*this); }

      uint8_t           readoutSpeed()      const     { return _readoutSpeed; }
      uint8_t           triggerMode()       const     { return _triggerMode; }
      int32_t           timepixSpeed()      const     { return _timepixSpeed; }

      int32_t           dac0Ikrum()        const     { return _dac0Ikrum; }
      int32_t           dac0Disc()         const     { return _dac0Disc; }
      int32_t           dac0Preamp()       const     { return _dac0Preamp; }
      int32_t           dac0BufAnalogA()   const     { return _dac0BufAnalogA; }
      int32_t           dac0BufAnalogB()   const     { return _dac0BufAnalogB; }
      int32_t           dac0Hist()         const     { return _dac0Hist; }
      int32_t           dac0ThlFine()      const     { return _dac0ThlFine; }
      int32_t           dac0ThlCourse()    const     { return _dac0ThlCourse; }
      int32_t           dac0Vcas()         const     { return _dac0Vcas; }
      int32_t           dac0Fbk()          const     { return _dac0Fbk; }
      int32_t           dac0Gnd()          const     { return _dac0Gnd; }
      int32_t           dac0Ths()          const     { return _dac0Ths; }
      int32_t           dac0BiasLvds()     const     { return _dac0BiasLvds; }
      int32_t           dac0RefLvds()      const     { return _dac0RefLvds; }

      int32_t           dac1Ikrum()        const     { return _dac1Ikrum; }
      int32_t           dac1Disc()         const     { return _dac1Disc; }
      int32_t           dac1Preamp()       const     { return _dac1Preamp; }
      int32_t           dac1BufAnalogA()   const     { return _dac1BufAnalogA; }
      int32_t           dac1BufAnalogB()   const     { return _dac1BufAnalogB; }
      int32_t           dac1Hist()         const     { return _dac1Hist; }
      int32_t           dac1ThlFine()      const     { return _dac1ThlFine; }
      int32_t           dac1ThlCourse()    const     { return _dac1ThlCourse; }
      int32_t           dac1Vcas()         const     { return _dac1Vcas; }
      int32_t           dac1Fbk()          const     { return _dac1Fbk; }
      int32_t           dac1Gnd()          const     { return _dac1Gnd; }
      int32_t           dac1Ths()          const     { return _dac1Ths; }
      int32_t           dac1BiasLvds()     const     { return _dac1BiasLvds; }
      int32_t           dac1RefLvds()      const     { return _dac1RefLvds; }

      int32_t           dac2Ikrum()        const     { return _dac2Ikrum; }
      int32_t           dac2Disc()         const     { return _dac2Disc; }
      int32_t           dac2Preamp()       const     { return _dac2Preamp; }
      int32_t           dac2BufAnalogA()   const     { return _dac2BufAnalogA; }
      int32_t           dac2BufAnalogB()   const     { return _dac2BufAnalogB; }
      int32_t           dac2Hist()         const     { return _dac2Hist; }
      int32_t           dac2ThlFine()      const     { return _dac2ThlFine; }
      int32_t           dac2ThlCourse()    const     { return _dac2ThlCourse; }
      int32_t           dac2Vcas()         const     { return _dac2Vcas; }
      int32_t           dac2Fbk()          const     { return _dac2Fbk; }
      int32_t           dac2Gnd()          const     { return _dac2Gnd; }
      int32_t           dac2Ths()          const     { return _dac2Ths; }
      int32_t           dac2BiasLvds()     const     { return _dac2BiasLvds; }
      int32_t           dac2RefLvds()      const     { return _dac2RefLvds; }

      int32_t           dac3Ikrum()        const     { return _dac3Ikrum; }
      int32_t           dac3Disc()         const     { return _dac3Disc; }
      int32_t           dac3Preamp()       const     { return _dac3Preamp; }
      int32_t           dac3BufAnalogA()   const     { return _dac3BufAnalogA; }
      int32_t           dac3BufAnalogB()   const     { return _dac3BufAnalogB; }
      int32_t           dac3Hist()         const     { return _dac3Hist; }
      int32_t           dac3ThlFine()      const     { return _dac3ThlFine; }
      int32_t           dac3ThlCourse()    const     { return _dac3ThlCourse; }
      int32_t           dac3Vcas()         const     { return _dac3Vcas; }
      int32_t           dac3Fbk()          const     { return _dac3Fbk; }
      int32_t           dac3Gnd()          const     { return _dac3Gnd; }
      int32_t           dac3Ths()          const     { return _dac3Ths; }
      int32_t           dac3BiasLvds()     const     { return _dac3BiasLvds; }
      int32_t           dac3RefLvds()      const     { return _dac3RefLvds; }

      int32_t           chipCount()        const     { return ChipCount; }

      void              driverVersion(int32_t version);
      int32_t           driverVersion() const;
      void              firmwareVersion(uint32_t version);
      uint32_t          firmwareVersion() const;
      void              pixelThresh(uint32_t size, uint8_t *thresh);
      uint32_t          pixelThreshSize() const;
      const uint8_t *   pixelThresh() const;
      void              chip0Name(const char *name);
      const char *      chip0Name() const;
      void              chip1Name(const char *name);
      const char *      chip1Name() const;
      void              chip2Name(const char *name);
      const char *      chip2Name() const;
      void              chip3Name(const char *name);
      const char *      chip3Name() const;
      void              chip0ID(int32_t id)          { _chip0ID = id; }
      int32_t           chip0ID()           const    { return _chip0ID; }
      void              chip1ID(int32_t id)          { _chip1ID = id; }
      int32_t           chip1ID()           const    { return _chip1ID; }
      void              chip2ID(int32_t id)          { _chip2ID = id; }
      int32_t           chip2ID()           const    { return _chip2ID; }
      void              chip3ID(int32_t id)          { _chip3ID = id; }
      int32_t           chip3ID()           const    { return _chip3ID; }

      // misc
      void              dump() const;

private:
      int8_t      _readoutSpeed, _triggerMode;
      int32_t     _timepixSpeed;

      int32_t     _dac0Ikrum, _dac0Disc, _dac0Preamp;
      int32_t     _dac0BufAnalogA, _dac0BufAnalogB, _dac0Hist;
      int32_t     _dac0ThlFine, _dac0ThlCourse, _dac0Vcas;
      int32_t     _dac0Fbk, _dac0Gnd, _dac0Ths;
      int32_t     _dac0BiasLvds, _dac0RefLvds;

      int32_t     _dac1Ikrum, _dac1Disc, _dac1Preamp;
      int32_t     _dac1BufAnalogA, _dac1BufAnalogB, _dac1Hist;
      int32_t     _dac1ThlFine, _dac1ThlCourse, _dac1Vcas;
      int32_t     _dac1Fbk, _dac1Gnd, _dac1Ths;
      int32_t     _dac1BiasLvds, _dac1RefLvds;

      int32_t     _dac2Ikrum, _dac2Disc, _dac2Preamp;
      int32_t     _dac2BufAnalogA, _dac2BufAnalogB, _dac2Hist;
      int32_t     _dac2ThlFine, _dac2ThlCourse, _dac2Vcas;
      int32_t     _dac2Fbk, _dac2Gnd, _dac2Ths;
      int32_t     _dac2BiasLvds, _dac2RefLvds;

      int32_t     _dac3Ikrum, _dac3Disc, _dac3Preamp;
      int32_t     _dac3BufAnalogA, _dac3BufAnalogB, _dac3Hist;
      int32_t     _dac3ThlFine, _dac3ThlCourse, _dac3Vcas;
      int32_t     _dac3Fbk, _dac3Gnd, _dac3Ths;
      int32_t     _dac3BiasLvds, _dac3RefLvds;

      int32_t     _driverVersion;
      uint32_t    _firmwareVersion;
      uint32_t    _pixelThreshSize;
      uint8_t     _pixelThresh[PixelThreshMax];
      char        _chip0Name[ChipNameMax+1];
      char        _chip1Name[ChipNameMax+1];
      char        _chip2Name[ChipNameMax+1];
      char        _chip3Name[ChipNameMax+1];
      int32_t     _chip0ID;
      int32_t     _chip1ID;
      int32_t     _chip2ID;
      int32_t     _chip3ID;
    };
  };
};

#pragma pack()

#endif
