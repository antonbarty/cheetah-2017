#ifndef TIMEPIXDATATYPE_HH
#define TIMEPIXDATATYPE_HH

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/timepix/DataV2.hh"

typedef Pds::Timepix::DataV2 TimepixDataType;

static Pds::TypeId _timepixDataType(Pds::TypeId::Id_TimepixData,
                                    TimepixDataType::Version);

#endif
