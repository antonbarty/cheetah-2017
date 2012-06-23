#include "pdsdata/evr/IOChannel.hh"

#include <string.h>

using namespace Pds::EvrData;

IOChannel::IOChannel() {}

IOChannel::IOChannel(const char* name,
		     const Pds::DetInfo* infos, 
		     unsigned    ninfo) :
  _ninfo(ninfo)
{
  strncpy(_name,name,NameLength);
  _ninfo = ninfo;
  for(unsigned i=0; i<ninfo; i++)
    _info[i] = *infos++;
}

IOChannel::IOChannel(const IOChannel& c) :
  _ninfo(c._ninfo)
{
  strncpy(_name,c._name,NameLength);
  for(unsigned i=0; i<c._ninfo; i++)
    _info[i] = c._info[i];
}

unsigned IOChannel::ninfo() const { return _ninfo; }

const Pds::DetInfo& IOChannel::info(unsigned i) const { return _info[i]; }

const char*         IOChannel::name() const { return _name; }
  
unsigned IOChannel::size() const { return sizeof(*this); }
