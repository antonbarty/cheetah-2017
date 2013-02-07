#ifndef Pds_PimImageConfigType_hh
#define Pds_PimImageConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/lusi/PimImageConfigV1.hh"

typedef Pds::Lusi::PimImageConfigV1 PimImageConfigType;

static Pds::TypeId _pimImageConfigType(Pds::TypeId::Id_PimImageConfig,
				       PimImageConfigType::Version);

#endif
