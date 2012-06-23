#ifndef Pds_IpimbConfigType_hh
#define Pds_IpimbConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/ipimb/ConfigV2.hh"

typedef Pds::Ipimb::ConfigV2 IpimbConfigType;

static Pds::TypeId _ipimbConfigType(Pds::TypeId::Id_IpimbConfig,
				  IpimbConfigType::Version);

#endif
