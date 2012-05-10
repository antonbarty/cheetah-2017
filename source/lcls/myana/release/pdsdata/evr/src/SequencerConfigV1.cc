#include "pdsdata/evr/SequencerConfigV1.hh"
#include "pdsdata/evr/SequencerEntry.hh"

#include <string.h>

using namespace Pds::EvrData;

SequencerConfigV1::SequencerConfigV1(Source   sync_source,
				     Source   beam_source,
				     unsigned length,
				     unsigned cycles,
				     const SequencerEntry* entries) :
  _source     ( ((sync_source&0xff)<< 0) |
                ((beam_source&0xff)<< 8) ),
  _length     (length),
  _cycles     (cycles)
{
  SequencerEntry* e = reinterpret_cast<SequencerEntry*>(this+1);
  for(unsigned i=0; i<length; i++)
    *e++ = *entries++;
}

SequencerConfigV1::SequencerConfigV1(const SequencerConfigV1& c) 
{
  memcpy(this, &c, c.size());
}

SequencerConfigV1::Source SequencerConfigV1::sync_source() const 
{ return Source(_source&0xff); }

SequencerConfigV1::Source SequencerConfigV1::beam_source() const
{ return Source((_source&0xff00)>>8); }

unsigned SequencerConfigV1::length     () const { return _length; }
unsigned SequencerConfigV1::cycles     () const { return _cycles; }
const SequencerEntry& SequencerConfigV1::entry(unsigned i) const
{ return reinterpret_cast<const SequencerEntry*>(this+1)[i]; }

unsigned SequencerConfigV1::size() const 
{ return sizeof(*this) + _length*sizeof(SequencerEntry); }
