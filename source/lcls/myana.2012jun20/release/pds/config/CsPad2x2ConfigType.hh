/*
 * CsPad2x2ConfigType.hh
 *
 *  Created on: Jan 10, 2012
 */

#ifndef CSPAD2x2CONFIGTYPE_HH_
#define CSPAD2x2CONFIGTYPE_HH_

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/cspad2x2/ConfigV1.hh"

typedef Pds::CsPad2x2::ConfigV1 CsPad2x2ConfigType;

static Pds::TypeId _CsPad2x2ConfigType(Pds::TypeId::Id_Cspad2x2Config,
                                    CsPad2x2ConfigType::Version);


#endif /* CSPAD2x2CONFIGTYPE_HH_ */
