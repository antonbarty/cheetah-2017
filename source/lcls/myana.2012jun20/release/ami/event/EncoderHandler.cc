#include "EncoderHandler.hh"

#include "ami/data/FeatureCache.hh"

#include "pdsdata/encoder/ConfigV1.hh"
#include "pdsdata/encoder/DataV2.hh"
#include "pdsdata/xtc/DetInfo.hh"

#include <stdio.h>
#include <string.h>

using namespace Ami;

EncoderHandler::EncoderHandler(const Pds::DetInfo& info, FeatureCache& f) :
  EventHandler(info,
	       Pds::TypeId::Id_EncoderData,
	       Pds::TypeId::Id_EncoderConfig),
  _cache(f)
{
}

EncoderHandler::~EncoderHandler()
{
}

void   EncoderHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void   EncoderHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  char buffer[64];
  strncpy(buffer,Pds::DetInfo::name(static_cast<const Pds::DetInfo&>(info())),59);
  char* c = buffer+strlen(buffer);

  sprintf(c,":CH0");
  _index = _cache.add(buffer);
  sprintf(c,":CH1");
  _cache.add(buffer);
  sprintf(c,":CH2");
  _cache.add(buffer);
}

void   EncoderHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  const Pds::Encoder::DataV2& d = *reinterpret_cast<const Pds::Encoder::DataV2*>(payload);
  _cache.cache(_index+0, d.value(0));
  _cache.cache(_index+1, d.value(1));
  _cache.cache(_index+2, d.value(2));
}

void   EncoderHandler::_damaged  ()
{
  _cache.cache(_index, 0, true);
}

//  No Entry data
unsigned     EncoderHandler::nentries() const { return 0; }
const Entry* EncoderHandler::entry   (unsigned) const { return 0; }
void         EncoderHandler::reset   () 
{
}
