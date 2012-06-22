#include "DiodeFexHandler.hh"

#include "ami/data/FeatureCache.hh"

#include "pdsdata/lusi/DiodeFexV1.hh"
#include "pdsdata/xtc/DetInfo.hh"

#include <stdio.h>
#include <string.h>

using namespace Ami;

DiodeFexHandler::DiodeFexHandler(const Pds::DetInfo& info, FeatureCache& f) :
  EventHandler(info,
	       Pds::TypeId::Id_DiodeFex,
	       Pds::TypeId::Id_DiodeFexConfig),
  _cache(f)
{
}

DiodeFexHandler::~DiodeFexHandler()
{
}

void   DiodeFexHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void   DiodeFexHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  char buffer[64];

  strncpy(buffer,Pds::DetInfo::name(static_cast<const Pds::DetInfo&>(info())),60);
  char* iptr = buffer+strlen(buffer);
  
  sprintf(iptr,":CH0"); 
  _index = _cache.add(buffer);
}

void   DiodeFexHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  const Pds::Lusi::DiodeFexV1& d = *reinterpret_cast<const Pds::Lusi::DiodeFexV1*>(payload);

  _cache.cache(_index, d.value);
}

void   DiodeFexHandler::_damaged  ()
{
  _cache.cache(_index, 0, true);
}

//  No Entry data
unsigned     DiodeFexHandler::nentries() const { return 0; }
const Entry* DiodeFexHandler::entry   (unsigned) const { return 0; }
void         DiodeFexHandler::reset   () 
{
}
