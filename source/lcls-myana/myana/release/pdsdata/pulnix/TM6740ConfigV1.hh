//
//  Class for configuration of Pulnix TM6740CL monochrome camera
//
#ifndef Pulnix_TM6740ConfigV1_hh
#define Pulnix_TM6740ConfigV1_hh

#include <stdint.h>

namespace Pds {

  namespace Pulnix {

    class TM6740ConfigV1 {
    public:
      enum { Version=1 };
      enum Depth     { Eight_bit, Ten_bit };
      enum Binning   { x1, x2, x4 };
      enum LookupTable { Gamma, Linear };
      enum { Row_Pixels = 480 };
      enum { Column_Pixels = 640 };

      TM6740ConfigV1();
      //  hard-code video output order (mode 'c')
      //            shutter mode (pulse width control 'AS9')
      TM6740ConfigV1(unsigned  black_level,  // 0 - 0x1ff [10-bit scale]
		     unsigned  gaina,        // 0x42 - 0x1e8 [66-232] (6 - 22 dB)
		     unsigned  gainb,
		     unsigned  shutter_width_us,
		     bool      gain_balance,
		     Depth     depth,
		     Binning   hbinning,
		     Binning   vbinning,
		     LookupTable     lut );

      //
      //  Accessors
      //

      //  offset/pedestal setting for camera (before gain)
      unsigned short   vref  () const;
      unsigned short   gain_a() const;
      unsigned short   gain_b() const;

      bool             gain_balance() const;
      
      //  shutter width in microseconds
      unsigned short   shutter_width() const;

      //  bit-depth of pixel counts
      Depth            output_resolution() const;
      unsigned         output_resolution_bits() const;

      //  horizontal re-binning of output (consecutive columns summed)
      Binning          horizontal_binning() const;

      //  vertical re-binning of output (consecutive rows summed)
      Binning          vertical_binning() const;

      //  apply output lookup table corrections
      LookupTable      lookuptable_mode() const;

    private:
      uint32_t _gain_a_b;      // gains
      uint32_t _vref_shutter;  // offset and shutter width
      uint32_t _control;       // output options
    };

  };
};

#endif
