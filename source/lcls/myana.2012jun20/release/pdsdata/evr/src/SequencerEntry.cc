#include "pdsdata/evr/SequencerEntry.hh"

using namespace Pds::EvrData;

SequencerEntry::SequencerEntry(unsigned eventcode,
			       unsigned delay) :
  _value( ((eventcode&0xff)<<24) | (delay&0xffffff) )
{
}

unsigned SequencerEntry::eventcode() const { return _value>>24; }
unsigned SequencerEntry::delay    () const { return _value&0xffffff; }
