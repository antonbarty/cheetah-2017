#include "pdsdata/gsc16ai/DataV1.hh"

uint16_t Pds::Gsc16ai::DataV1::channelValue(int chan) const
{
  return (_channelValue[chan]);
}

uint16_t Pds::Gsc16ai::DataV1::timestamp(int index) const
{
  return (_timestamp[index]);
}
