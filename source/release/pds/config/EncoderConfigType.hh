#ifndef ENCODERCONFIGTYPE_HH
#define ENCODERCONFIGTYPE_HH

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/encoder/ConfigV2.hh"

typedef Pds::Encoder::ConfigV2 EncoderConfigType;

static Pds::TypeId _encoderConfigType(Pds::TypeId::Id_EncoderConfig,
                                      EncoderConfigType::Version);

#endif
