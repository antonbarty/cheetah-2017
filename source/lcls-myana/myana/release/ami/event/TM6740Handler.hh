#ifndef Ami_TM6740Handler_hh
#define Ami_TM6740Handler_hh

#include "ami/event/FrameHandler.hh"
#include "pdsdata/pulnix/TM6740ConfigV1.hh"
#include "pdsdata/lusi/PimImageConfigV1.hh"

namespace Ami {
  class TM6740Handler : public FrameHandler {
  public:
    TM6740Handler(const Pds::DetInfo& info);
  public:
    void   _configure(Pds::TypeId, 
		      const void* payload, const Pds::ClockTime& t);
    void   _configure(const void* payload, const Pds::ClockTime& t);
    void   _event    (const void* payload, const Pds::ClockTime& t);
  private:
    Pds::Lusi::PimImageConfigV1 _scale;
  };
};

#endif
