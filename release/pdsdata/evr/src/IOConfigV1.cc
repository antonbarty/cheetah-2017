#include "pdsdata/evr/IOConfigV1.hh"

#include "pdsdata/evr/IOChannel.hh"

using namespace Pds::EvrData;

IOConfigV1::IOConfigV1 () : _conn(0) {}
IOConfigV1::IOConfigV1 (OutputMap::Conn  conn,
			const IOChannel* channels,
			unsigned         nchannels) :
  _conn(unsigned(conn)),
  _nchannels(nchannels)
{
  IOChannel* c = reinterpret_cast<IOChannel*>(this+1);
  for(unsigned i=0; i<nchannels; i++)
    *c++ = channels[i];
}

OutputMap::Conn IOConfigV1::conn() const { return OutputMap::Conn(_conn); }

unsigned IOConfigV1::nchannels() const { return _nchannels; }

const IOChannel& IOConfigV1::channel(unsigned i) const 
{ return reinterpret_cast<const IOChannel*>(this+1)[i]; }

unsigned IOConfigV1::size() const 
{ return sizeof(*this) + _nchannels*sizeof(IOChannel); }
