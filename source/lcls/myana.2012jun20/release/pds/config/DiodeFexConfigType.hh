#ifndef Pds_DiodeFexConfigType_hh
#define Pds_DiodeFexConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/lusi/DiodeFexConfigV2.hh"

typedef Pds::Lusi::DiodeFexConfigV2 DiodeFexConfigType;

static Pds::TypeId _diodeFexConfigType(Pds::TypeId::Id_DiodeFexConfig,
				       DiodeFexConfigType::Version);

#endif
