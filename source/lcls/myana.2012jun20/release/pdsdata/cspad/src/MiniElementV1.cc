#include "pdsdata/cspad/MiniElementV1.hh"

using namespace Pds::CsPad;

MiniElementV1::MiniElementV1() 
{
}

uint16_t MiniElementV1::pixel(unsigned asic,
                              unsigned col,
                              unsigned row) const
{
  unsigned r = (asic&1)*MaxRowsPerASIC+row;
  return (asic&2) ? pair[col][r].s1 : pair[col][r].s0;
}
