#include "pdsdata/acqiris/TdcConfigV1.hh"

using namespace Pds::Acqiris;

TdcChannel::TdcChannel() {}

TdcChannel::TdcChannel(Channel  chan,
		       Mode     mode,
		       Slope    slope,
		       double   level) :
  _channel(chan),
  _mode   ((unsigned(mode)<<31) |
	   (unsigned(slope)&1)),
  _level  (level)
{
}

TdcChannel::Channel  TdcChannel::channel()      const
{ return Channel(_channel); }
TdcChannel::Mode     TdcChannel::mode   ()      const
{ return Mode(_mode>>31); }
TdcChannel::Slope    TdcChannel::slope  ()      const
{ return Slope(_mode&1); }
double               TdcChannel::level  ()      const
{ return _level; }



TdcAuxIO::TdcAuxIO() {}

TdcAuxIO::TdcAuxIO(Channel     channel,
		   Mode        mode,
		   Termination term) :
  _channel  (channel),
  _signal   (mode),
  _qualifier(term)
{
}

TdcAuxIO::Channel     TdcAuxIO::channel() const
{ return Channel(_channel); }
TdcAuxIO::Mode        TdcAuxIO::mode   () const
{ return Mode(_signal); }
TdcAuxIO::Termination TdcAuxIO::term   () const
{ return Termination(_qualifier); }



TdcVetoIO::TdcVetoIO() {}

TdcVetoIO::TdcVetoIO(Mode        mode,
		     Termination term) :
  _channel  (ChVeto),
  _signal   (mode),
  _qualifier(term)
{
}

TdcVetoIO::Channel     TdcVetoIO::channel() const
{ return Channel(_channel); }
TdcVetoIO::Mode        TdcVetoIO::mode   () const
{ return Mode(_signal); }
TdcVetoIO::Termination TdcVetoIO::term   () const
{ return Termination(_qualifier); }


TdcConfigV1::TdcConfigV1 () {}

TdcConfigV1::~TdcConfigV1 () {}

TdcConfigV1::TdcConfigV1 (const TdcChannel channels[],
			  const TdcAuxIO   auxio[],
			  const TdcVetoIO& veto)
{
  for(unsigned i=0; i<NChannels; i++)
    _channel[i] = channels[i];
  for(unsigned i=0; i<NAuxIO; i++)
    _auxIO[i] = auxio[i];
  _veto = veto;
}

const TdcChannel& TdcConfigV1::channel(unsigned i) const
{ return _channel[i]; }
const TdcAuxIO&   TdcConfigV1::auxio  (unsigned i) const
{ return _auxIO[i]; }
const TdcVetoIO&  TdcConfigV1::veto   ()         const
{ return _veto; }
