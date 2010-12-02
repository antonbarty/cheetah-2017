#include "pdsdata/encoder/DataV2.hh"

int Pds::Encoder::DataV2::value(int chan) const
{
  int32_t v(_encoder_count[chan]<<8);
  return v>>8;
  return _encoder_count[chan];
}

