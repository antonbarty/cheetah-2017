#include "IpimbHandler.hh"

#include "ami/data/FeatureCache.hh"

#include "pdsdata/ipimb/DataV1.hh"
#include "pdsdata/ipimb/DataV2.hh"
#include "pdsdata/xtc/DetInfo.hh"

#include <stdio.h>
#include <string.h>

using namespace Ami;

IpimbHandler::IpimbHandler(const Pds::DetInfo& info, FeatureCache& f) :
  EventHandler(info,
	       Pds::TypeId::Id_IpimbData,
	       Pds::TypeId::Id_IpimbConfig),
  _cache (f)
{
}

IpimbHandler::~IpimbHandler()
{
}

void   IpimbHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void   IpimbHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  char buffer[64];

  strncpy(buffer,Pds::DetInfo::name(static_cast<const Pds::DetInfo&>(info())),60);
  char* iptr = buffer+strlen(buffer);
  
  for(unsigned i=0; i<NChannels; i++) {
    sprintf(iptr,"-Ch%d",i);
    _index[i] = _cache.add(buffer);
  }
}

void   IpimbHandler::_event    (Pds::TypeId id,
                                const void* payload, const Pds::ClockTime& t)
{
  if (id.version()==1) {
    const Pds::Ipimb::DataV1& d = 
      *reinterpret_cast<const Pds::Ipimb::DataV1*>(payload);
    _cache.cache(_index[0], d.channel0Volts());
    _cache.cache(_index[1], d.channel1Volts());
    _cache.cache(_index[2], d.channel2Volts());
    _cache.cache(_index[3], d.channel3Volts());
  }
  else if (id.version()==2) {
    const Pds::Ipimb::DataV2& d = 
      *reinterpret_cast<const Pds::Ipimb::DataV2*>(payload);
    _cache.cache(_index[0], d.channel0Volts());
    _cache.cache(_index[1], d.channel1Volts());
    _cache.cache(_index[2], d.channel2Volts());
    _cache.cache(_index[3], d.channel3Volts());
  }
  else 
    ;
}

void   IpimbHandler::_damaged  ()
{
  _cache.cache(_index[0], 0, true);
  _cache.cache(_index[1], 0, true);
  _cache.cache(_index[2], 0, true);
  _cache.cache(_index[3], 0, true);
}

//  No Entry data
unsigned     IpimbHandler::nentries() const { return 0; }
const Entry* IpimbHandler::entry   (unsigned) const { return 0; }
void         IpimbHandler::reset   () 
{
}
