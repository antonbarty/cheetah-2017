#ifndef Pds_EvrIOConfigType_hh
#define Pds_EvrIOConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/evr/IOConfigV1.hh"
#include "pdsdata/evr/IOChannel.hh"

typedef Pds::EvrData::IOConfigV1  EvrIOConfigType;

static Pds::TypeId _evrIOConfigType(Pds::TypeId::Id_EvrIOConfig,
				    EvrIOConfigType::Version);

#endif
