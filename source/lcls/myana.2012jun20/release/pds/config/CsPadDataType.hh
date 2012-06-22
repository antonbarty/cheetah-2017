#ifndef CSPADDATATYPE_HH
#define CSPADDATATYPE_HH

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/cspad/ElementV2.hh"

typedef Pds::CsPad::ElementV2 CsPadDataType;

static Pds::TypeId _CsPadDataType(Pds::TypeId::Id_CspadElement,
                                    Pds::CsPad::ElementV2::Version);

#endif
