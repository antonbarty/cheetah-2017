#ifndef Pds_OceanOpticsConfigType_hh
#define Pds_OceanOpticsConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/oceanoptics/ConfigV1.hh"

typedef Pds::OceanOptics::ConfigV1 OceanOpticsConfigType;

static Pds::TypeId _oceanOpticsConfigType(Pds::TypeId::Id_OceanOpticsConfig,
          OceanOpticsConfigType::Version);

#endif
