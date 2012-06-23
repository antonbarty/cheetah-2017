#ifndef Pds_FrameFccdConfigType_hh
#define Pds_FrameFccdConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/camera/FrameFccdConfigV1.hh"

typedef Pds::Camera::FrameFccdConfigV1 FrameFccdConfigType;

static Pds::TypeId _frameFccdConfigType(Pds::TypeId::Id_FrameFccdConfig,
				       FrameFccdConfigType::Version);


#endif
