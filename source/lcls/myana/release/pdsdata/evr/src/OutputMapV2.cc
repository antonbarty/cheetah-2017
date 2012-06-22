#include "pdsdata/evr/OutputMapV2.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

OutputMapV2::OutputMapV2 () {}

OutputMapV2::OutputMapV2 (Source src , unsigned source_id,
			  Conn   conn, unsigned conn_id,
			  unsigned module)
{
  _v = (((unsigned(src)&0x0f) << 0) | 
	((    source_id&0xff) << 4) | 
	((unsigned(conn)&0x0f)<<12) | 
	((      conn_id&0xff) <<16) |
	((       module&0xff) <<24));
}

OutputMapV2::Source OutputMapV2::source() const
{ return (Source)(_v&0x0f); }

unsigned OutputMapV2::source_id() const
{ return (_v>>4)&0xff; }

OutputMapV2::Conn OutputMapV2::conn() const
{ return (Conn)((_v>>12)&0x0f); }

unsigned OutputMapV2::conn_id() const
{ return (_v>>16)&0xff; }

unsigned OutputMapV2::module() const
{ return (_v>>24)&0xff; }

unsigned OutputMapV2::map() const
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
