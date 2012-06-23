#include "pdsdata/cspad/ElementV1.hh"
#include "pdsdata/cspad/ConfigV1.hh"

using namespace Pds::CsPad;

ElementV1::ElementV1() 
{
}

const uint16_t* ElementV1::data() const
{
  return reinterpret_cast<const uint16_t*>(this+1);
}

const uint16_t* ElementV1::pixel(unsigned asic,
				 unsigned col,
				 unsigned row) const
{
  const uint16_t* d = data();     // quadrant
  d += ColumnsPerASIC*MaxRowsPerASIC*2*(asic>>1);  // advance to 2x1
  d += col*MaxRowsPerASIC*2;                // advance to column
  d += MaxRowsPerASIC*(asic&1);             // advance to ASIC
  d += row;                       // advance to row
  return d;
}

const ElementV1* ElementV1::next(const ConfigV1& c) const
{
  return reinterpret_cast<const ElementV1*>
    ( reinterpret_cast<const uint16_t*>(this+1)+
      c.numAsicsRead()*ColumnsPerASIC*MaxRowsPerASIC + 2 );
}
