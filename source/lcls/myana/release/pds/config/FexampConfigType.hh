/*
 * XampsConfigType.hh
 *
 *  Created on: Apr 28, 2011
 */

#ifndef FEXAMP_CONFIGTYPE_HH_
#define FEXAMP_CONFIGTYPE_HH_

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/fexamp/ConfigV1.hh"
#include "pdsdata/fexamp/ASIC_V1.hh"
#include "pdsdata/fexamp/ChannelV1.hh"

typedef Pds::Fexamp::ConfigV1  FexampConfigType;
typedef Pds::Fexamp::ASIC_V1   FexampASIC;
typedef Pds::Fexamp::ChannelV1 FexampChannel;

static Pds::TypeId _FexampConfigType(Pds::TypeId::Id_FexampConfig,
                                    FexampConfigType::Version);


#endif /* FEXAMP_CONFIGTYPE_HH_ */
