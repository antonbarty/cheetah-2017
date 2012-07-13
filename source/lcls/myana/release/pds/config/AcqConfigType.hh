#ifndef Pds_AcqConfigType_hh
#define Pds_AcqConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/TdcConfigV1.hh"

typedef Pds::Acqiris::ConfigV1 AcqConfigType;
typedef Pds::Acqiris::TdcConfigV1 AcqTdcConfigType;

static Pds::TypeId _acqConfigType(Pds::TypeId::Id_AcqConfig,
				  AcqConfigType::Version);
static Pds::TypeId _acqTdcConfigType(Pds::TypeId::Id_AcqTdcConfig,
				     AcqTdcConfigType::Version);

#endif
