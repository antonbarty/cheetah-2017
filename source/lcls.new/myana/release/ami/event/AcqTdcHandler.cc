#include "AcqTdcHandler.hh"

#include "ami/data/EntryRef.hh"
#include "ami/data/ChannelID.hh"
#include "pds/config/AcqDataType.hh"
#include "pdsdata/xtc/ClockTime.hh"

#include <stdio.h>

static const unsigned MaxHits = 1000;
static const double TDC_PERIOD = 50e-12;
using namespace Ami;

AcqTdcHandler::AcqTdcHandler(const Pds::DetInfo& info) : 
  EventHandler(info, Pds::TypeId::Id_AcqTdcData, Pds::TypeId::Id_AcqTdcConfig),
  _entry(NULL)
{
}

AcqTdcHandler::AcqTdcHandler(const Pds::DetInfo&     info, 
			     const AcqTdcConfigType& config) :
  EventHandler(info, Pds::TypeId::Id_AcqTdcData, Pds::TypeId::Id_AcqTdcConfig),
  _entry(NULL)
{
  Pds::ClockTime t;
  _configure(&config, t);
}

AcqTdcHandler::~AcqTdcHandler()
{
}

unsigned AcqTdcHandler::nentries() const { return (_entry != NULL); }

const Entry* AcqTdcHandler::entry(unsigned i) const { return (i == 0 ? _entry : NULL); }

void AcqTdcHandler::reset() { _entry = NULL; }

void AcqTdcHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}

void AcqTdcHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  const Pds::DetInfo& det = static_cast<const Pds::DetInfo&>(info());
  _entry = new EntryRef(DescRef(det,0,ChannelID::name(det,0),"Hits"));
}

void AcqTdcHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  _entry->set(payload);
  _entry->valid(t);
}

void AcqTdcHandler::_damaged() 
{
  _entry->invalid();
}
