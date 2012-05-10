#include "OceanOpticsHandler.hh"

#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryRef.hh"
#include "ami/data/ChannelID.hh"
#include "pds/config/OceanOpticsConfigType.hh"
#include "pds/config/OceanOpticsDataType.hh"
#include "pdsdata/xtc/ClockTime.hh"

#include <stdio.h>

using namespace Ami;

static OceanOpticsConfigType _default(0.001);

OceanOpticsHandler::OceanOpticsHandler(const Pds::DetInfo& info) : 
  EventHandler(info, Pds::TypeId::Id_OceanOpticsData, Pds::TypeId::Id_OceanOpticsConfig),
  _config(_default),
  _nentries(0)
{
}

OceanOpticsHandler::OceanOpticsHandler(const Pds::DetInfo&   info, 
               const OceanOpticsConfigType& config) :
  EventHandler(info, Pds::TypeId::Id_OceanOpticsData, Pds::TypeId::Id_OceanOpticsConfig),
  _config(_default),
  _nentries(0)
{
  Pds::ClockTime t;
  _configure(&config, t);
}

OceanOpticsHandler::~OceanOpticsHandler()
{
}

unsigned OceanOpticsHandler::nentries() const { return _nentries; }

const Entry* OceanOpticsHandler::entry(unsigned i) const 
{
  if (i<_nentries)
    return _entry[i];
  else
    return NULL;
}

void OceanOpticsHandler::reset() {
  _nentries = 0;
}

void OceanOpticsHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}

void OceanOpticsHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  const OceanOpticsConfigType& c = *reinterpret_cast<const OceanOpticsConfigType*>(payload);

  unsigned  channelNumber = 0;  
  double    fMinX         = OceanOpticsDataType::waveLength1stOrder(c, 0);
  double    fMaxX         = OceanOpticsDataType::waveLength1stOrder(c, OceanOpticsDataType::iNumPixels - 1);
  
  const Pds::DetInfo& det = static_cast<const Pds::DetInfo&>(info());
  DescWaveform desc(det, channelNumber,ChannelID::name(det,channelNumber),
    "Wavelength [m]","Count", 3840, fMinX, fMaxX);
    
  _entry[_nentries++] = new EntryWaveform(desc);
  _config = c;
}

void OceanOpticsHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  OceanOpticsDataType* d = (OceanOpticsDataType*)payload;
  
  EntryWaveform* entry  = _entry[0];
  
  for (int j=0;j<OceanOpticsDataType::iNumPixels;j++)
    entry->content( d->nonlinerCorrected(_config, j),j);
    
  entry->info(1,EntryWaveform::Normalization);
  entry->valid(t);
}

void OceanOpticsHandler::_damaged() 
{
  for (unsigned i=0;i<_nentries;i++) {
    EntryWaveform* entry = _entry[i];
    entry->invalid();
  }
}
