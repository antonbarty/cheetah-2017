
#include "pdsdata/acqiris/DataDescV1.hh"

using namespace Pds;
using namespace Acqiris;

static const unsigned Version=1;

Pds::TypeId DataDescV1::typeId() {
  return Pds::TypeId(Pds::TypeId::Id_AcqConfig,Version);
}

double   TimestampV1::pos  () const { return _horPos; }

uint64_t TimestampV1::value() const {
  uint64_t ts = _timeStampHi;
  ts = (ts<<32) + (unsigned long)(_timeStampLo);
  return ts;
}

uint64_t TimestampV1::operator-(const TimestampV1& ts) const {
  return value()-ts.value();
}

uint32_t DataDescV1::nbrSamplesInSeg() const {
  return _returnedSamplesPerSeg;
}

uint32_t DataDescV1::nbrSegments() const {
  return _returnedSegments;
}

TimestampV1& DataDescV1::timestamp(uint32_t seg) {
  return _timestamp()[seg];
}

int16_t* DataDescV1::waveform(const HorizV1& hconfig) {
  return (int16_t*)(&(_timestamp()[hconfig.nbrSegments()]));
}

uint32_t DataDescV1::indexFirstPoint() {
  return _indexFirstPoint;
}

uint32_t DataDescV1::timestampSize(const HorizV1& hconfig) {
  return hconfig.nbrSegments()*sizeof(TimestampV1);}

uint32_t DataDescV1::waveformSize(const HorizV1& hconfig) {
  return hconfig.nbrSamples()*hconfig.nbrSegments()*sizeof(short)+_extra;}

uint32_t DataDescV1::totalSize(const HorizV1& hconfig) {
  return sizeof(DataDescV1)+DataDescV1::timestampSize(hconfig)+DataDescV1::waveformSize(hconfig);}

DataDescV1* DataDescV1::nextChannel(const HorizV1& hconfig) {
  return (DataDescV1*)((char*)(waveform(hconfig))+DataDescV1::waveformSize(hconfig));
}

TimestampV1* DataDescV1::_timestamp() {
  return (TimestampV1*)(this+1);
}
