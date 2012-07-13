#ifndef Pds_Gsc16aiConfigType_hh
#define Pds_Gsc16aiConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/gsc16ai/ConfigV1.hh"

typedef Pds::Gsc16ai::ConfigV1 Gsc16aiConfigType;

static Pds::TypeId _gsc16aiConfigType(Pds::TypeId::Id_Gsc16aiConfig,
                                      Gsc16aiConfigType::Version);

#endif
