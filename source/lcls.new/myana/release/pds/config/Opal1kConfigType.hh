#ifndef Pds_Opal1kConfigType_hh
#define Pds_Opal1kConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/opal1k/ConfigV1.hh"

typedef Pds::Opal1k::ConfigV1 Opal1kConfigType;

static Pds::TypeId _opal1kConfigType(Pds::TypeId::Id_Opal1kConfig,
				     Opal1kConfigType::Version);

#endif
