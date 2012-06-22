#include "pdsdata/lusi/IpmFexV1.hh"

using namespace Pds::Lusi;

IpmFexV1::IpmFexV1(float ch[], float chsum,
		   float x, float y) 
{
  channel[0] = ch[0];
  channel[1] = ch[1];
  channel[2] = ch[2];
  channel[3] = ch[3];
  sum  = chsum;
  xpos = x;
  ypos = y;
}
