#ifndef FEXAMPDATATYPE_HH
#define FEXAMPDATATYPE_HH

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/fexamp/ElementV1.hh"

typedef Pds::Fexamp::ElementV1 FexampDataType;

static Pds::TypeId _FexampDataType(Pds::TypeId::Id_FexampElement,
                                    Pds::Fexamp::ElementV1::Version);

#endif
