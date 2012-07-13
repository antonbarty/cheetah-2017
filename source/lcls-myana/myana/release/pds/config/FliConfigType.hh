#ifndef Pds_FliConfigType_hh
#define Pds_FliConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/fli/ConfigV1.hh"

typedef Pds::Fli::ConfigV1 FliConfigType;

static Pds::TypeId _fliConfigType(Pds::TypeId::Id_FliConfig,
          FliConfigType::Version);

#endif
