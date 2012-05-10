/*
 * XampsConfigType.hh
 *
 *  Created on: Apr 28, 2011
 */

#ifndef XAMPSCONFIGTYPE_HH_
#define XAMPSCONFIGTYPE_HH_

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xamps/ConfigV1.hh"
#include "pdsdata/xamps/ASIC_V1.hh"
#include "pdsdata/xamps/ChannelV1.hh"

typedef Pds::Xamps::ConfigV1  XampsConfigType;
typedef Pds::Xamps::ASIC_V1   XampsASIC;
typedef Pds::Xamps::ChannelV1 XampsChannel;

static Pds::TypeId _XampsConfigType(Pds::TypeId::Id_XampsConfig,
                                    XampsConfigType::Version);


#endif /* XAMPSCONFIGTYPE_HH_ */
