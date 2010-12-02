#include "pdsdata/pulnix/TM6740ConfigV2.hh"

using namespace Pds::Pulnix;

enum { Gain_Bal_Shift = 0,
       Depth_Shift    = 1,
       HBin_Shift     = 2,
       VBin_Shift     = 4,
       LUT_Shift      = 6 };

TM6740ConfigV2::TM6740ConfigV2() {}

TM6740ConfigV2::TM6740ConfigV2(unsigned  vref_a,  // 0 - 0x3ff
			       unsigned  vref_b,  // 0 - 0x3ff
			       unsigned  gaina,  // 0x42 - 0x1e8 [66-232]
			       unsigned  gainb,
			       bool      gain_balance,
			       Depth     depth,
			       Binning   hbinning,
			       Binning   vbinning,
			       LookupTable lut ) :
  _gain_a_b       ( (gainb<<16) | (gaina&0xffff) ),
  _vref_shutter   ( ((vref_a&0x3ff)<<0) | ((vref_b&0x3ff)<<16) ),
  _control        ( (gain_balance ? (1<<Gain_Bal_Shift) : 0) |
		    (unsigned(depth)<<Depth_Shift) |
		    (unsigned(hbinning)<<HBin_Shift) |
		    (unsigned(vbinning)<<VBin_Shift) |
		    (unsigned(lut)<<LUT_Shift) )
{}

unsigned short   TM6740ConfigV2::vref_a() const { return (_vref_shutter>> 0)&0x3ff; }
unsigned short   TM6740ConfigV2::vref_b() const { return (_vref_shutter>>16)&0x3ff; }
unsigned short   TM6740ConfigV2::gain_a() const { return _gain_a_b&0xffff; }
unsigned short   TM6740ConfigV2::gain_b() const { return _gain_a_b>>16; }

bool             TM6740ConfigV2::gain_balance() const { return _control & (1<<Gain_Bal_Shift); }
      
//  bit-depth of pixel counts
TM6740ConfigV2::Depth TM6740ConfigV2::output_resolution() const { return TM6740ConfigV2::Depth((_control>>Depth_Shift)&1); }
unsigned         TM6740ConfigV2::output_resolution_bits() const { return (_control & (1<<Depth_Shift)) ? 10 : 8; }

//  horizontal re-binning of output (consecutive columns summed)
TM6740ConfigV2::Binning TM6740ConfigV2::horizontal_binning() const 
{ return TM6740ConfigV2::Binning((_control>>HBin_Shift)&3); }

//  vertical re-binning of output (consecutive rows summed)
TM6740ConfigV2::Binning TM6740ConfigV2::vertical_binning() const
{ return TM6740ConfigV2::Binning((_control>>VBin_Shift)&3); }

//  apply output lookup table corrections
TM6740ConfigV2::LookupTable TM6740ConfigV2::lookuptable_mode() const 
{ return TM6740ConfigV2::LookupTable((_control>>LUT_Shift)&1); }
