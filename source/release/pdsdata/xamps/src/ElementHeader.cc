#include "pdsdata/xamps/ElementHeader.hh"

using namespace Pds::Xamps;

ElementHeader::ElementHeader() 
{
}

unsigned ElementHeader::virtual_channel    () const 
{
  return _word0&3; 
}

unsigned ElementHeader::lane               () const 
{
  return (_word0>>6)&3; 
}

unsigned ElementHeader::tid                () const 
{
  return _word0>>8; 
}

unsigned ElementHeader::acq_count          () const 
{
  return _word1&0xffff; 
}

unsigned ElementHeader::op_code            () const
{
  return (_word1>>16)&0xff;
}

unsigned ElementHeader::elementId          () const
{
  return (_word1>>24)&3; 
}

unsigned ElementHeader::seq_count          () const 
{
  return _seq_count; 
}

//unsigned ElementHeader::sb_temp            (unsigned i) const
//{
//  return _sbtemp[i];
//}

unsigned ElementHeader::frame_type         () const
{
  return _frame_type&0xff; 
}

//unsigned ElementHeader::ticks() const
//{
//  return _ticks;
//}
//
//unsigned ElementHeader::fiducials() const
//{
//  return _fiducials;
//}
