#include "pdsdata/encoder/DataV2.hh"

int Pds::Encoder::DataV2::value(int chan) const
{
  int32_t v(_encoder_count[chan]<<8);
  return v>>8;
}



Pds::Encoder::DataV2::DataV2()
{
	_33mhz_timestamp = 0;
	_encoder_count[0] = 0;
	_encoder_count[1] = 0;
	_encoder_count[2] = 0;
}


Pds::Encoder::DataV2::DataV2(uint32_t timestamp, uint32_t count0, uint32_t count1, uint32_t count2)
{
	_33mhz_timestamp = timestamp;
	_encoder_count[0] = count0;
	_encoder_count[1] = count1;
	_encoder_count[2] = count2;
}


Pds::Encoder::DataV2::DataV2(uint32_t timestamp, uint32_t count0)
{
	_33mhz_timestamp = timestamp;
	_encoder_count[0] = count0;  /* This was '0' in the original..? */
	_encoder_count[1] = 0;
	_encoder_count[2] = 0;
}