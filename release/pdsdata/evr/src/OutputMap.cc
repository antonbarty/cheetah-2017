#include "pdsdata/evr/OutputMap.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

OutputMap::OutputMap () {}

OutputMap::OutputMap (Source src , unsigned source_id,
		      Conn   conn, unsigned conn_id)
{
  _v = ((unsigned(src) << 0) | 
	(    source_id << 8) | 
	(unsigned(conn)<<16) | 
	(      conn_id <<24));
}

OutputMap::Source OutputMap::source() const
{ return (Source)(_v&0xff); }

unsigned OutputMap::source_id() const
{ return (_v>>8)&0xff; }

OutputMap::Conn OutputMap::conn() const
{ return (Conn)((_v>>16)&0xff); }

unsigned OutputMap::conn_id() const
{ return _v>>24; }

unsigned OutputMap::map() const
{
  enum { Pulse_Offset=0, 
	 DBus_Offset=32, 
	 Prescaler_Offset=40 }; 
  unsigned map=0;
  Source src    = source();
  unsigned    src_id =source_id();
  switch(src) {
  case Pulse     : map = (src_id + Pulse_Offset); break;
  case DBus      : map = (src_id + DBus_Offset); break;
  case Prescaler : map = (src_id + Prescaler_Offset); break;
  case Force_High: map = 62; break;
  case Force_Low : map = 63; break;
  }
  return map;
}
