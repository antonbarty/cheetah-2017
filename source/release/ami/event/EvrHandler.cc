#include "EvrHandler.hh"

#include "ami/data/FeatureCache.hh"

#include "pdsdata/evr/ConfigV5.hh"
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/evr/PulseConfigV3.hh"
#include "pdsdata/xtc/DetInfo.hh"

#include <stdio.h>
#include <string.h>
#include <math.h>

using namespace Ami;

EvrHandler::EvrHandler(const Pds::DetInfo& info, FeatureCache& f) :
  EventHandler(info,
	       Pds::TypeId::Id_EvrData,
	       Pds::TypeId::Id_EvrConfig),
  _cache(f)
{
}

EvrHandler::~EvrHandler()
{
}

void   EvrHandler::_calibrate(Pds::TypeId type, const void* payload, const Pds::ClockTime& t) 
{ _configure(type,payload,t); }

#define REGISTER_CODE(evtcode) {		\
    unsigned code = evtcode;			\
    sprintf(iptr,"Evt%d",code);			\
    int index = _cache.add(buffer);		\
    _cache.cache(index, 0);			\
    _index[code] = index;			\
  }

void   EvrHandler::_configure(Pds::TypeId, const void* payload, const Pds::ClockTime& t)
{
  const Pds::EvrData::ConfigV5& c = *reinterpret_cast<const Pds::EvrData::ConfigV5*>(payload);

  char buffer[64];

  sprintf(buffer,"DAQ:EVR%d:",static_cast<const Pds::DetInfo&>(info()).devId());
  char* iptr = buffer+strlen(buffer);

  for(unsigned i=0; i<c.npulses(); i++) {
    sprintf(iptr,"P%d:Delay",i);
    int index = _cache.add(buffer);
    double delay = floor(c.pulse(i).delay()*c.pulse(i).prescale())/119.e6;
    _cache.cache(index, delay);
  }

  memset(_index, -1, sizeof(_index));

  for(unsigned i=0; i<c.neventcodes(); i++) {
    REGISTER_CODE(c.eventcode(i).code());
  }

  REGISTER_CODE(140);
  REGISTER_CODE(162);
}

#undef REGISTER_CODE

void   EvrHandler::_event    (Pds::TypeId type, const void* payload, const Pds::ClockTime& t) 
{
  for(unsigned i=0; i<256; i++)
    if (_index[i]>=0)
      _cache.cache(_index[i],0);

  if (type.version()==3) {
    const Pds::EvrData::DataV3& d = *reinterpret_cast<const Pds::EvrData::DataV3*>(payload);

    for(unsigned i=0; i<d.numFifoEvents(); i++) {
      int index = _index[d.fifoEvent(i).EventCode];
      if (index >=0) _cache.cache(index, 1);
    }
  }
}

void   EvrHandler::_damaged  () 
{
  for(unsigned i=0; i<256; i++)
    if (_index[i]>=0)
      _cache.cache(_index[i],0);
}

//  No Entry data
unsigned     EvrHandler::nentries() const { return 0; }
const Entry* EvrHandler::entry   (unsigned) const { return 0; }
void         EvrHandler::reset   () {}
