#ifndef Pds_FliDataType_hh
#define Pds_FliDataType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/fli/FrameV1.hh"

typedef Pds::Fli::FrameV1 FliDataType;

static Pds::TypeId _fliDataType(Pds::TypeId::Id_FliFrame,
          FliDataType::Version);

#endif
