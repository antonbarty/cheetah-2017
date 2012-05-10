#ifndef CSPAD2x2DATATYPE_HH
#define CSPAD2x2DATATYPE_HH

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/cspad2x2/ElementV1.hh"

typedef Pds::CsPad2x2::ElementV1 CsPad2x2DataType;

static Pds::TypeId _CsPad2x2DataType(Pds::TypeId::Id_Cspad2x2Element,
                                    Pds::CsPad2x2::ElementV1::Version);

#endif
