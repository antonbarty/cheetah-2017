#include "pdsdata/cspad2x2/ElementV1.hh"

using namespace Pds::CsPad2x2;

ElementV1::ElementV1()
{
}

uint16_t ElementV1::pixel(unsigned asic,
                              unsigned col,
                              unsigned row) const
{
  unsigned r = (asic&1)*MaxRowsPerASIC+row;
  return (asic&2) ? pair[col][r].s1 : pair[col][r].s0;
}
