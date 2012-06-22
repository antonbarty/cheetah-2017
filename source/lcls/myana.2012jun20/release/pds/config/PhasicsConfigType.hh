/*
 * PhasicsConfigType.hh
 *
 *  Created on: Dec 7, 2011
 */

#ifndef PHASICS_CONFIGTYPE_HH_
#define PHASICS_CONFIGTYPE_HH_

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/phasics/ConfigV1.hh"

typedef Pds::Phasics::ConfigV1  PhasicsConfigType;

static Pds::TypeId _PhasicsConfigType(Pds::TypeId::Id_PhasicsConfig,
                                    PhasicsConfigType::Version);


#endif /* PHASICS_CONFIGTYPE_HH_ */
