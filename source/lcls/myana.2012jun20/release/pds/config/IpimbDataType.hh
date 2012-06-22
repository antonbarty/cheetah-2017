#ifndef Pds_IpimbDataType_hh
#define Pds_IpimbDataType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/ipimb/DataV2.hh"

typedef Pds::Ipimb::DataV2 IpimbDataType;

static Pds::TypeId _ipimbDataType(Pds::TypeId::Id_IpimbData,
				  IpimbDataType::Version);

#endif
