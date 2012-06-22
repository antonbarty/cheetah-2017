#ifndef Pds_IpmFexConfigType_hh
#define Pds_IpmFexConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/lusi/IpmFexConfigV2.hh"

typedef Pds::Lusi::IpmFexConfigV2 IpmFexConfigType;

static Pds::TypeId _ipmFexConfigType(Pds::TypeId::Id_IpmFexConfig,
				     IpmFexConfigType::Version);

#endif
