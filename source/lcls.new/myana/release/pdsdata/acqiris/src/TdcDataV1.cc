#include "pdsdata/acqiris/TdcDataV1.hh"

using namespace Pds::Acqiris;

static const double TdcPeriod = 50e-12;  // 50 picosecond resolution


TdcDataV1::Source TdcDataV1::source() const { return Source((_value>>28)&0x7); }


bool     TdcDataV1::Common::overflow() const { return _value>>31; }

unsigned TdcDataV1::Common::nhits   () const { return _value&0xfffffff; }


bool     TdcDataV1::Channel::overflow() const { return _value>>31; }

unsigned TdcDataV1::Channel::ticks   () const { return _value&0xfffffff; }

double   TdcDataV1::Channel::time    () const { return double(_value&0xfffffff)*TdcPeriod; }


TdcDataV1::Marker::Type TdcDataV1::Marker::type() const { return Type(_value&0xfffffff); }
