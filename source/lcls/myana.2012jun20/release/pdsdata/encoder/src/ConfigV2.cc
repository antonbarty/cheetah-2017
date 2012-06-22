#include "pdsdata/encoder/ConfigV2.hh"
#include <stdio.h>
#include <stdint.h>

using namespace Pds;
using namespace Encoder;

static const unsigned Version = 2;

// These duplicated from the PCI-3E driver code.  Sorry!

static const char* count_mode_to_name[] = {
   "COUNT_MODE_WRAP_FULL",
   "COUNT_MODE_LIMIT",
   "COUNT_MODE_HALT",
   "COUNT_MODE_WRAP_PRESET"
};

static const char* quad_mode_to_name[] = {
   "QUAD_MODE_CLOCK_DIR",
   "QUAD_MODE_X1",
   "QUAD_MODE_X2",
   "QUAD_MODE_X4"
};

void Pds::Encoder::ConfigV2::dump() const
{
  printf( ">>------ Encoder Config -----------\n" );
  printf( "\tChannel mask: 0x%x\n", _chan_mask );
  printf( "\tEncoder counting mode: %u (%s)\n",
          _count_mode,
          count_mode_to_name[_count_mode] );
  printf( "\tEncoder quadrature mode: %u (%s)\n",
          _quadrature_mode,
          quad_mode_to_name[_quadrature_mode] );
  printf( "\tExternal input for trigger: %u\n", _input_num );
  printf( "\tTrigger on edge: %u (%s)\n",
          _input_rising,
          _input_rising ? "Rising" : "Falling" );
  printf( "\tEncoder timebase ticks per second: %u\n", _ticks_per_sec );
  printf( "<<------ End Encoder Config -----------\n" );
}

Pds::Encoder::ConfigV2::ConfigV2( uint32_t           chan_mask,
                                  uint32_t           count_mode,
                                  uint32_t           quadrature_mode,
                                  uint32_t           input_num,
                                  uint32_t           input_rising,
                                  uint32_t           ticks_per_sec ) 
   : _chan_mask(        chan_mask ),
     _count_mode(      count_mode ),
     _quadrature_mode( quadrature_mode ),
     _input_num(       input_num ),
     _input_rising(    input_rising ),
     _ticks_per_sec(   ticks_per_sec ) 
{}
