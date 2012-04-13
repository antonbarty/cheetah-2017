#include "FrameFexHandler.hh"

#include "ami/data/FeatureCache.hh"

#include "pdsdata/camera/TwoDGaussianV1.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"
#include "pdsdata/xtc/DetInfo.hh"

#include <stdio.h>
#include <string.h>

using namespace Ami;

FrameFexHandler::FrameFexHandler(const Pds::DetInfo& info, FeatureCache& f) :
  EventHandler(info,
	       Pds::TypeId::Id_TwoDGaussian,
	       Pds::TypeId::Id_FrameFexConfig),
  _cache(f)
{
}

FrameFexHandler::~FrameFexHandler()
{
}

void   FrameFexHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void   FrameFexHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  const Pds::Camera::FrameFexConfigV1& d = *reinterpret_cast<const Pds::Camera::FrameFexConfigV1*>(payload);
  if (d.processing() == Pds::Camera::FrameFexConfigV1::NoProcessing) {
    _index[0] = -1;
    return;
  }

  char buffer[64];

  strncpy(buffer,Pds::DetInfo::name(static_cast<const Pds::DetInfo&>(info())),60);
  char* iptr = buffer+strlen(buffer);
  
  unsigned i=0;
  sprintf(iptr,":INT"); _index[i++] = _cache.add(buffer);
  sprintf(iptr,":X"  ); _index[i++] = _cache.add(buffer);
  sprintf(iptr,":Y"  ); _index[i++] = _cache.add(buffer);
  sprintf(iptr,":MAJ"); _index[i++] = _cache.add(buffer);
  sprintf(iptr,":MIN"); _index[i++] = _cache.add(buffer);
  sprintf(iptr,":PHI"); _index[i++] = _cache.add(buffer);
}

void   FrameFexHandler::_event    (const void* payload, const Pds::ClockTime& t)
{
  const Pds::Camera::TwoDGaussianV1& d = *reinterpret_cast<const Pds::Camera::TwoDGaussianV1*>(payload);

  unsigned i=0;
  _cache.cache(_index[i++], d.integral());
  _cache.cache(_index[i++], d.xmean());
  _cache.cache(_index[i++], d.ymean());
  _cache.cache(_index[i++], d.major_axis_width());
  _cache.cache(_index[i++], d.minor_axis_width());
  _cache.cache(_index[i++], d.major_axis_tilt());
}

void   FrameFexHandler::_damaged  ()
{
  if (_index[0] >= 0)
    for(unsigned i=0; i<NChannels; i++)
      _cache.cache(_index[i], 0, true);
}

//  No Entry data
unsigned     FrameFexHandler::nentries() const { return 0; }
const Entry* FrameFexHandler::entry   (unsigned) const { return 0; }
void         FrameFexHandler::reset   () 
{
}
