#ifndef Pds_OceanOpticsDataType_hh
#define Pds_OceanOpticsDataType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/oceanoptics/DataV1.hh"

typedef Pds::OceanOptics::DataV1 OceanOpticsDataType;

static Pds::TypeId _oceanOpticsDataType(Pds::TypeId::Id_OceanOpticsData,
          OceanOpticsDataType::Version);

#endif
