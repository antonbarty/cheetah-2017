#ifndef Pds_FliDataType_hh
#define Pds_FliDataType_hh

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/princeton/FrameV1.hh"

typedef Pds::Princeton::FrameV1 PrincetonDataType;

static Pds::TypeId _princetonDataType(Pds::TypeId::Id_PrincetonFrame,
          PrincetonDataType::Version);

#endif
