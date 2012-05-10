#ifndef XAMPSDATATYPE_HH
#define XAMPSDATATYPE_HH

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xamps/ElementV1.hh"

typedef Pds::Xamps::ElementV1 XampsDataType;

static Pds::TypeId _XampsDataType(Pds::TypeId::Id_XampsElement,
                                    Pds::Xamps::ElementV1::Version);

#endif
