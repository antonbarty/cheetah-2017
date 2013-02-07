#include "pdsdata/pulnix/TM6740ConfigV1.hh"

using namespace Pds::Pulnix;

enum { Gain_Bal_Shift = 0,
       Depth_Shift    = 1,
       HBin_Shift     = 2,
       VBin_Shift     = 4,
       LUT_Shift      = 6 };

TM6740ConfigV1::TM6740ConfigV1() {}

TM6740ConfigV1::TM6740ConfigV1(unsigned  vref ,  // 0 - 0x1ff
			       unsigned  gaina,  // 0x42 - 0x1e8 [66-232]
			       unsigned  gainb,
			       unsigned  shutter_width,
			       bool      gain_balance,
			       Depth     depth,
			       Binning   hbinning,
			       Binning   vbinning,
			       LookupTable lut ) :
  _gain_a_b       ( (gainb<<16) | (gaina&0xffff) ),
  _vref_shutter   ( (vref&0xffff) | (shutter_width<<16) ),
  _control        ( (gain_balance ? (1<<Gain_Bal_Shift) : 0) |
		    (unsigned(depth)<<Depth_Shift) |
		    (unsigned(hbinning)<<HBin_Shift) |
		    (unsigned(vbinning)<<VBin_Shift) |
		    (unsigned(lut)<<LUT_Shift) )
{}

unsigned short   TM6740ConfigV1::vref  () const { return _vref_shutter&0xffff; }
unsigned short   TM6740ConfigV1::gain_a() const { return _gain_a_b&0xffff; }
unsigned short   TM6740ConfigV1::gain_b() const { return _gain_a_b>>16; }
unsigned short   TM6740ConfigV1::shutter_width() const { return _vref_shutter>>16; }

bool             TM6740ConfigV1::gain_balance() const { return _control & (1<<Gain_Bal_Shift); }
      
//  bit-depth of pixel counts
TM6740ConfigV1::Depth TM6740ConfigV1::output_resolution() const { return TM6740ConfigV1::Depth((_control>>Depth_Shift)&1); }
unsigned         TM6740ConfigV1::output_resolution_bits() const { return (_control & (1<<Depth_Shift)) ? 10 : 8; }

//  horizontal re-binning of output (consecutive columns summed)
TM6740ConfigV1::Binning TM6740ConfigV1::horizontal_binning() const 
{ return TM6740ConfigV1::Binning((_control>>HBin_Shift)&3); }

//  vertical re-binning of output (consecutive rows summed)
TM6740ConfigV1::Binning TM6740ConfigV1::vertical_binning() const
{ return TM6740ConfigV1::Binning((_control>>VBin_Shift)&3); }

//  apply output lookup table corrections
TM6740ConfigV1::LookupTable TM6740ConfigV1::lookuptable_mode() const 
{ return TM6740ConfigV1::LookupTable((_control>>LUT_Shift)&1); }
