#include "pdsdata/xtc/TypeId.hh"

using namespace Pds;

TypeId::TypeId(Type type, uint32_t version) :
  _value((version<<16)|type) {}

TypeId::TypeId(const TypeId& v) : _value(v._value) {}

uint32_t TypeId::value() const {return _value;}

uint32_t TypeId::version() const {return (_value&0xffff0000)>>16;}

TypeId::Type TypeId::id() const {return (TypeId::Type)(_value&0xffff);}

const char* TypeId::name(Type type)
{ 
   static const char* _names[NumberOf] = {
    "Any",
    "Xtc",
    "Frame",
    "AcqWaveform",
    "AcqConfig",
    "TwoDGaussian",
    "Opal1kConfig",
    "FrameFexConfig",
    "EvrConfig",
    "TM6740Config",
    "RunControlConfig",
    "pnCCDframe",
    "pnCCDconfig",
    "Epics",    
    "FEEGasDetEnergy",
    "EBeamBld",
    "PhaseCavity",
    "PrincetonFrame",
    "PrincetonConfig",    
    "EvrData",
    "FrameFccdConfig",
    "FccdConfig",  
    "IpimbData",
    "IpimbConfig",
    "EncoderData",
    "EncoderConfig",
    "EvrIOConfig",
    "PrincetonInfo",    
    "CspadElement",
    "CspadConfig",
    "IpmFexConfig",
    "IpmFex",
    "DiodeFexConfig",
    "DiodeFex",
    "PimImageConfig",
    "SharedIpimb",
    "AcqTDCConfig",
    "AcqTDCData",
    "Index",
    "XampsConfig",
    "XampsElement"
  };
  return (type < NumberOf ? _names[type] : "-Invalid-");
}
