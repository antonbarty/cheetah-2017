#ifndef Pds_TM6740ConfigType_hh
#define Pds_TM6740ConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/pulnix/TM6740ConfigV2.hh"

typedef Pds::Pulnix::TM6740ConfigV2 TM6740ConfigType;

static Pds::TypeId _tm6740ConfigType(Pds::TypeId::Id_TM6740Config,
				     TM6740ConfigType::Version);

#endif
