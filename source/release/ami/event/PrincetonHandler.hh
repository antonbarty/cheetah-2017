#ifndef Ami_PrincetonHandler_hh
#define Ami_PrincetonHandler_hh

#include "ami/event/EventHandler.hh"
#include "pdsdata/princeton/ConfigV1.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {
  class EntryImage;

  class PrincetonHandler : public EventHandler {
  public:
    PrincetonHandler(const Pds::DetInfo& info);
    ~PrincetonHandler();
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
    PrincetonHandler(const Pds::DetInfo& info, const EntryImage*);
    Pds::Princeton::ConfigV1 _config;
    EntryImage* _entry;
  };
};

#endif
