#include "pdsdata/timepix/DataV1.hh"

uint32_t Pds::Timepix::DataV1::timestamp(void) const
{
  return (_timestamp);
}

uint16_t Pds::Timepix::DataV1::frameCounter(void) const
{
  return (_frameCounter);
}

uint16_t Pds::Timepix::DataV1::lostRows(void) const
{
  return (_lostRows);
}
