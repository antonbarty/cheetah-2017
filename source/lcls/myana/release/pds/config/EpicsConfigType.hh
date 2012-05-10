#ifndef Pds_EpicsConfigType_hh
#define Pds_EpicsConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/epics/ConfigV1.hh"

typedef Pds::Epics::ConfigV1 EpicsConfigType;

static Pds::TypeId _epicsConfigType(Pds::TypeId::Id_EpicsConfig,
          EpicsConfigType::Version);

#endif
