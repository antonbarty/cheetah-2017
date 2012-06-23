//
//  Class for configuration of LBNL/ANL FCCD monochrome camera
//
#ifndef Fccd_ConfigV2_hh
#define Fccd_ConfigV2_hh

#include <stdint.h>

#pragma pack(4)

#define FCCD_DAC1_V_START   0.0
#define FCCD_DAC1_V_END     9.0
#define FCCD_DAC1_LABEL     "V(1-3) pos (0.0->9.0)"

#define FCCD_DAC2_V_START   0.0
#define FCCD_DAC2_V_END     -9.0
#define FCCD_DAC2_LABEL     "V(1-3) neg (-9.0->0.0)"

#define FCCD_DAC3_V_START   0.0
#define FCCD_DAC3_V_END     9.0
#define FCCD_DAC3_LABEL     "TgClk pos (0.0->9.0)"

#define FCCD_DAC4_V_START   0.0
#define FCCD_DAC4_V_END     -9.0
#define FCCD_DAC4_LABEL     "TgClk neg (-9.0->0.0)"

#define FCCD_DAC5_V_START   0.0
#define FCCD_DAC5_V_END     9.0
#define FCCD_DAC5_LABEL     "H(1-3) pos (0.0->9.0)"

#define FCCD_DAC6_V_START   0.0
#define FCCD_DAC6_V_END     -9.0
#define FCCD_DAC6_LABEL     "H(1-3) neg (-9.0->0.0)"

#define FCCD_DAC7_V_START   0.0
#define FCCD_DAC7_V_END     9.0
#define FCCD_DAC7_LABEL     "SwtClk pos (0.0->9.0)"

#define FCCD_DAC8_V_START   0.0
#define FCCD_DAC8_V_END     -9.0
#define FCCD_DAC8_LABEL     "SwtClk neg (-9.0->0.0)"

#define FCCD_DAC9_V_START   0.0
#define FCCD_DAC9_V_END     9.0
#define FCCD_DAC9_LABEL     "RgClk pos (0.0->9.0)"

#define FCCD_DAC10_V_START  0.0
#define FCCD_DAC10_V_END    -9.0
#define FCCD_DAC10_LABEL    "RgClk neg (-9.0->0.0)"

#define FCCD_DAC11_V_START  0.0
#define FCCD_DAC11_V_END    99.0
#define FCCD_DAC11_LABEL    "HV (0.0->99.0)"

#define FCCD_DAC12_V_START  0.0
#define FCCD_DAC12_V_END    5.0
#define FCCD_DAC12_LABEL    "OTG (0.0->5.0)"

#define FCCD_DAC13_V_START  0.0
#define FCCD_DAC13_V_END    -15.0
#define FCCD_DAC13_LABEL    "VDDRST (0.0->-15.0)"

#define FCCD_DAC14_V_START  0.0
#define FCCD_DAC14_V_END    -24.0
#define FCCD_DAC14_LABEL    "VDDOUT (0.0->-24.0)"

#define FCCD_DAC15_V_START  -10.0
#define FCCD_DAC15_V_END    10.0
#define FCCD_DAC15_LABEL    "NGD (-10.0->10.0)"

#define FCCD_DAC16_V_START  -10.0
#define FCCD_DAC16_V_END    10.0
#define FCCD_DAC16_LABEL    "NCON (-10.0->10.0)"

#define FCCD_DAC17_V_START  -10.0
#define FCCD_DAC17_V_END    10.0
#define FCCD_DAC17_LABEL    "GAURD (-10.0->10.0)"

namespace Pds {
  namespace Camera {
    class FrameCoord;
  };

  namespace FCCD {

    class FccdConfigV2 {
    public:
      enum { Version=2 };
      enum Depth          { Eight_bit=8, Sixteen_bit=16 };
      enum Output_Source  { Output_FIFO=0, Test_Pattern1=1, Test_Pattern2=2,
                            Test_Pattern3=3, Test_Pattern4=4 };
      // FCCD:
      //   Image size is 576 x 500 with 16 bit pixels (1152 x 500 with 8 bit pixels)
      enum { Row_Pixels=500 };
      enum { Column_Pixels=576 * 2};  // 1152 x 500 with 8 bit pixels
      //   After dropping dark pixels, size is 480 x 480 with 16 bit pixels
      enum { Trimmed_Row_Pixels=480 };
      enum { Trimmed_Column_Pixels=480 };

      FccdConfigV2() {}
      FccdConfigV2(
        uint16_t    outputMode,
        bool        ccdEnable,
        bool        focusMode,
        uint32_t    exposureTime,
        float       dacVoltage1,
        float       dacVoltage2,
        float       dacVoltage3,
        float       dacVoltage4,
        float       dacVoltage5,
        float       dacVoltage6,
        float       dacVoltage7,
        float       dacVoltage8,
        float       dacVoltage9,
        float       dacVoltage10,
        float       dacVoltage11,
        float       dacVoltage12,
        float       dacVoltage13,
        float       dacVoltage14,
        float       dacVoltage15,
        float       dacVoltage16,
        float       dacVoltage17,
        uint16_t    waveform0,
        uint16_t    waveform1,
        uint16_t    waveform2,
        uint16_t    waveform3,
        uint16_t    waveform4,
        uint16_t    waveform5,
        uint16_t    waveform6,
        uint16_t    waveform7,
        uint16_t    waveform8,
        uint16_t    waveform9,
        uint16_t    waveform10,
        uint16_t    waveform11,
        uint16_t    waveform12,
        uint16_t    waveform13,
        uint16_t    waveform14
      );

      uint32_t          width ()            const     { return Column_Pixels; }
      uint32_t          height()            const     { return Row_Pixels; }
      uint32_t          trimmedWidth ()     const     { return Trimmed_Column_Pixels; }
      uint32_t          trimmedHeight()     const     { return Trimmed_Row_Pixels; }
      unsigned          size()              const     { return (unsigned) sizeof(*this); }
      uint16_t          outputMode()        const     { return _outputMode; }
      bool              ccdEnable()         const     { return _ccdEnable; }
      bool              focusMode()         const     { return _focusMode; }
      uint32_t          exposureTime()      const     { return _exposureTime; }
      float             dacVoltage1()       const     { return _dacVoltage1; }
      float             dacVoltage2()       const     { return _dacVoltage2; }
      float             dacVoltage3()       const     { return _dacVoltage3; }
      float             dacVoltage4()       const     { return _dacVoltage4; }
      float             dacVoltage5()       const     { return _dacVoltage5; }
      float             dacVoltage6()       const     { return _dacVoltage6; }
      float             dacVoltage7()       const     { return _dacVoltage7; }
      float             dacVoltage8()       const     { return _dacVoltage8; }
      float             dacVoltage9()       const     { return _dacVoltage9; }
      float             dacVoltage10()      const     { return _dacVoltage10; }
      float             dacVoltage11()      const     { return _dacVoltage11; }
      float             dacVoltage12()      const     { return _dacVoltage12; }
      float             dacVoltage13()      const     { return _dacVoltage13; }
      float             dacVoltage14()      const     { return _dacVoltage14; }
      float             dacVoltage15()      const     { return _dacVoltage15; }
      float             dacVoltage16()      const     { return _dacVoltage16; }
      float             dacVoltage17()      const     { return _dacVoltage17; }
      uint16_t          waveform0()         const     { return _waveform0; }
      uint16_t          waveform1()         const     { return _waveform1; }
      uint16_t          waveform2()         const     { return _waveform2; }
      uint16_t          waveform3()         const     { return _waveform3; }
      uint16_t          waveform4()         const     { return _waveform4; }
      uint16_t          waveform5()         const     { return _waveform5; }
      uint16_t          waveform6()         const     { return _waveform6; }
      uint16_t          waveform7()         const     { return _waveform7; }
      uint16_t          waveform8()         const     { return _waveform8; }
      uint16_t          waveform9()         const     { return _waveform9; }
      uint16_t          waveform10()        const     { return _waveform10; }
      uint16_t          waveform11()        const     { return _waveform11; }
      uint16_t          waveform12()        const     { return _waveform12; }
      uint16_t          waveform13()        const     { return _waveform13; }
      uint16_t          waveform14()        const     { return _waveform14; }

private:
      uint16_t    _outputMode;
      bool        _ccdEnable;
      bool        _focusMode;
      uint32_t    _exposureTime;
      float       _dacVoltage1;
      float       _dacVoltage2;
      float       _dacVoltage3;
      float       _dacVoltage4;
      float       _dacVoltage5;
      float       _dacVoltage6;
      float       _dacVoltage7;
      float       _dacVoltage8;
      float       _dacVoltage9;
      float       _dacVoltage10;
      float       _dacVoltage11;
      float       _dacVoltage12;
      float       _dacVoltage13;
      float       _dacVoltage14;
      float       _dacVoltage15;
      float       _dacVoltage16;
      float       _dacVoltage17;
      uint16_t    _waveform0;
      uint16_t    _waveform1;
      uint16_t    _waveform2;
      uint16_t    _waveform3;
      uint16_t    _waveform4;
      uint16_t    _waveform5;
      uint16_t    _waveform6;
      uint16_t    _waveform7;
      uint16_t    _waveform8;
      uint16_t    _waveform9;
      uint16_t    _waveform10;
      uint16_t    _waveform11;
      uint16_t    _waveform12;
      uint16_t    _waveform13;
      uint16_t    _waveform14;
    };
  };
};

#pragma pack()

#endif
