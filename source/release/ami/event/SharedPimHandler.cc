#include "SharedPimHandler.hh"

#include "ami/data/Entry.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"

using namespace Ami;

typedef Pds::Camera::FrameFexConfigV1 FexConfigType;
static const FexConfigType _fexConfig(FexConfigType::FullFrame, 1,
                                      FexConfigType::NoProcessing,
                                      Pds::Camera::FrameCoord(0,0),
                                      Pds::Camera::FrameCoord(0,0),
                                      0, 0, 0);
static const Pds::TypeId camConfigType(Pds::TypeId::Id_TM6740Config,   Pds::Pulnix::TM6740ConfigV2::Version);
static const Pds::TypeId pimConfigType(Pds::TypeId::Id_PimImageConfig, Pds::Lusi::PimImageConfigV1::Version);
static const Pds::TypeId fexConfigType(Pds::TypeId::Id_FrameFexConfig, Pds::Camera::FrameFexConfigV1::Version);

SharedPimHandler::SharedPimHandler(const Pds::BldInfo& info) :
  EventHandler(info, Pds::TypeId::Id_SharedPim, Pds::TypeId::Id_SharedPim),
  _handler    (reinterpret_cast<const Pds::DetInfo&>(info))
{
}

void   SharedPimHandler::_configure(const void* payload, const Pds::ClockTime& t)
{
  const Pds::BldDataPimV1* bld = reinterpret_cast<const Pds::BldDataPimV1*>(payload);
  _handler._configure(camConfigType, &bld->camConfig, t);
  _handler._configure(pimConfigType, &bld->pimConfig, t);
  _handler._configure(fexConfigType, &_fexConfig    , t);
}

void   SharedPimHandler::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void   SharedPimHandler::_event    (const void* payload, const Pds::ClockTime& t) 
{
  const Pds::BldDataPimV1* bld = reinterpret_cast<const Pds::BldDataPimV1*>(payload);
  _handler._event(&bld->frame, t);
}
void   SharedPimHandler::_damaged() {} // { _handler.entry(0)->invalid(); }

void   SharedPimHandler::_configure(Pds::TypeId, 
                                    const void* payload, const Pds::ClockTime& t) { _configure(payload,t); }

unsigned     SharedPimHandler::nentries() const { return _handler.nentries(); }
const Entry* SharedPimHandler::entry            (unsigned i) const { return _handler.entry(i); }
void         SharedPimHandler::reset   () { _handler.reset(); }
