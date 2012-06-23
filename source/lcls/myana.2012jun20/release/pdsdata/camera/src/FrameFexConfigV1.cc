#include "pdsdata/camera/FrameFexConfigV1.hh"

#include <string.h>

using namespace Pds;
using namespace Camera;

FrameFexConfigV1::FrameFexConfigV1() {}

FrameFexConfigV1::FrameFexConfigV1( Forwarding        forwarding,
				    unsigned          fwd_prescale,
				    Processing        processing,
				    const FrameCoord& roiBegin,
				    const FrameCoord& roiEnd,
				    unsigned          threshold,
				    unsigned          masked_pixel_count,
				    const FrameCoord* masked_coords ) :
  _forwarding(forwarding),
  _forward_prescale(fwd_prescale),
  _processing(processing),
  _roiBegin  (roiBegin),
  _roiEnd    (roiEnd),
  _threshold (threshold),
  _masked_pixel_count (masked_pixel_count)
{
  if (masked_pixel_count)
    memcpy(this+1,masked_coords,masked_pixel_count*sizeof(FrameCoord));
}

FrameFexConfigV1::FrameFexConfigV1(const FrameFexConfigV1& c) :
  _forwarding(c._forwarding),
  _forward_prescale(c._forward_prescale),
  _processing(c._processing),
  _roiBegin  (c._roiBegin),
  _roiEnd    (c._roiEnd),
  _threshold (c._threshold),
  _masked_pixel_count (c._masked_pixel_count)
{
  if (c._masked_pixel_count)
    memcpy(this+1,&c+1,c._masked_pixel_count*sizeof(FrameCoord));
}

FrameFexConfigV1::Forwarding        FrameFexConfigV1::forwarding() const
{ return Forwarding(_forwarding); }

unsigned          FrameFexConfigV1::forward_prescale() const
{ return _forward_prescale; }

FrameFexConfigV1::Processing        FrameFexConfigV1::processing() const
{ return Processing(_processing); }

const FrameCoord& FrameFexConfigV1::roiBegin  () const
{ return _roiBegin; }

const FrameCoord& FrameFexConfigV1::roiEnd    () const
{ return _roiEnd; }

unsigned          FrameFexConfigV1::threshold () const
{ return _threshold; }

unsigned          FrameFexConfigV1::number_of_masked_pixels () const
{ return _masked_pixel_count; }

const FrameCoord& FrameFexConfigV1::masked_pixel_coordinates() const
{ return *reinterpret_cast<const FrameCoord*>(this+1); }

unsigned FrameFexConfigV1::size() const
{ return sizeof(*this) + _masked_pixel_count*sizeof(FrameCoord); }

