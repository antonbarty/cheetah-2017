#ifndef Pds_pnCCDConfigType_hh
#define Pds_pnCCDConfigType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/pnCCD/ConfigV2.hh"

typedef Pds::PNCCD::ConfigV2 pnCCDConfigType;

static Pds::TypeId _pnCCDConfigType(Pds::TypeId::Id_pnCCDconfig,
                                    pnCCDConfigType::Version);


#endif
