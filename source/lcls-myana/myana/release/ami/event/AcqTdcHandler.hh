#ifndef Ami_AcqTdcHandler_hh
#define Ami_AcqTdcHandler_hh

//===================================================================
//  AcqTdcHandler
//    Generates a 1D histogram with contents equal to the hit values
//===================================================================

#include "ami/event/EventHandler.hh"

#include "pds/config/AcqConfigType.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {
  class EntryRef;

  class AcqTdcHandler : public EventHandler {
  public:
    AcqTdcHandler(const Pds::DetInfo& info);
    ~AcqTdcHandler();
  public:
    unsigned     nentries() const;
    const Entry* entry(unsigned) const;
    void         reset();
  private:
    void _calibrate(const void* payload, const Pds::ClockTime& t);
    void _configure(const void* payload, const Pds::ClockTime& t);
    void _event    (const void* payload, const Pds::ClockTime& t);
    void _damaged  ();
  private:
    AcqTdcHandler(const Pds::DetInfo& info, 
		       const AcqTdcConfigType& config);
  private:
    AcqTdcConfigType _config;
    EntryRef*        _entry;
  };
};

#endif
