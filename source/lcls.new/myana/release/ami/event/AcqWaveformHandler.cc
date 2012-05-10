#include "AcqWaveformHandler.hh"

#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryRef.hh"
#include "ami/data/ChannelID.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/xtc/ClockTime.hh"

#include <stdio.h>

using namespace Ami;

static Pds::Acqiris::VertV1 _default_vert[Pds::Acqiris::ConfigV1::MaxChan];

static Pds::Acqiris::ConfigV1 _default(0,0,0,
				       Pds::Acqiris::TrigV1(0,0,0,0),
				       Pds::Acqiris::HorizV1(0,0,0,0),
				       _default_vert);

AcqWaveformHandler::AcqWaveformHandler(const Pds::DetInfo& info) : 
  EventHandler(info, Pds::TypeId::Id_AcqWaveform, Pds::TypeId::Id_AcqConfig),
  _config(_default),
  _nentries(0),
  _ref(NULL)
{
}

AcqWaveformHandler::AcqWaveformHandler(const Pds::DetInfo&   info, 
				       const Pds::Acqiris::ConfigV1& config) :
  EventHandler(info, Pds::TypeId::Id_AcqWaveform, Pds::TypeId::Id_AcqConfig),
  _config(_default),
  _nentries(0)
{
  Pds::ClockTime t;
  _configure(&config, t);
}

AcqWaveformHandler::~AcqWaveformHandler()
{
}

unsigned AcqWaveformHandler::nentries() const { return _nentries + (_ref != NULL); }

const Entry* AcqWaveformHandler::entry(unsigned i) const 
{
  if (i<_nentries)
    return _entry[i];
  else
    return _ref; 
}

void AcqWaveformHandler::reset() {
  _nentries = 0;
  _ref = NULL;
}

void AcqWaveformHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}

void AcqWaveformHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  const Pds::Acqiris::ConfigV1& c = *reinterpret_cast<const Pds::Acqiris::ConfigV1*>(payload);
  const Pds::Acqiris::HorizV1& h = c.horiz();
  unsigned channelMask = c.channelMask();
  unsigned channelNumber = 0;
  const Pds::DetInfo& det = static_cast<const Pds::DetInfo&>(info());
  for(unsigned k=0; channelMask!=0; k++) {
    if (channelMask&1) {
      DescWaveform desc(det, channelNumber,
			ChannelID::name(det,channelNumber),
			"Time [s]","Voltage [V]",
			h.nbrSamples(), 0., h.sampInterval()*h.nbrSamples());
      _entry[_nentries++] = new EntryWaveform(desc);
      channelNumber++;
    }
    channelMask >>= 1;
  }

  channelMask = c.channelMask() << 16;
  if (channelMask & (channelMask-1)) {
    char buff[32];
    sprintf(buff,"%p",_entry);
    _ref = new EntryRef(DescRef(det,channelMask,
                                ChannelID::name(det,channelMask),buff));
    _ref->set(_entry);
  }

  _config = c;
}

typedef Pds::Acqiris::DataDescV1 AcqDD;

void AcqWaveformHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  AcqDD* d = const_cast<AcqDD*>(reinterpret_cast<const AcqDD*>(payload));
  const Pds::Acqiris::HorizV1& h = _config.horiz();

  unsigned n = _nentries < _config.nbrChannels() ? _nentries : _config.nbrChannels();
  for (unsigned i=0;i<n;i++) {
    const int16_t* data = d->waveform(h);
    data += d->indexFirstPoint();
    float slope = _config.vert(i).slope();
    float offset = _config.vert(i).offset();
    EntryWaveform* entry = _entry[i];
    unsigned nbrSamples = h.nbrSamples();
    for (unsigned j=0;j<nbrSamples;j++) {
      int16_t swap = (data[j]&0xff<<8) | (data[j]&0xff00>>8);
      double val = swap*slope-offset;
      entry->content(val,j);
    }
    entry->info(1,EntryWaveform::Normalization);
    entry->valid(t);
    d = d->nextChannel(h);
  }

  if (_ref)
    _ref->valid(t);
}

void AcqWaveformHandler::_damaged() 
{
  unsigned n = _nentries < _config.nbrChannels() ? _nentries : _config.nbrChannels();
  for (unsigned i=0;i<n;i++) {
    EntryWaveform* entry = _entry[i];
    entry->invalid();
  }

  if (_ref)
    _ref->invalid();
}
