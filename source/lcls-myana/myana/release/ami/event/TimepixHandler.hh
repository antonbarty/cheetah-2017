#ifndef Ami_TimepixHandler_hh
#define Ami_TimepixHandler_hh

#include "ami/event/EventHandler.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {
  class EntryImage;

  class TimepixHandler : public EventHandler {
  public:
    TimepixHandler(const Pds::DetInfo& info);
    ~TimepixHandler();
  public:
    unsigned     nentries() const;
    const Entry* entry(unsigned) const;
    void         reset();
  protected:
    void _calibrate(Pds::TypeId type, const void* payload, const Pds::ClockTime& t);
    void _configure(Pds::TypeId type, const void* payload, const Pds::ClockTime& t);
    void _event    (Pds::TypeId type, const void* payload, const Pds::ClockTime& t);
    void _damaged  ();

    // The following methods are never called. See the TypeId variations above.
    void _calibrate(const void* payload, const Pds::ClockTime& t) { abort(); }
    void _configure(const void* payload, const Pds::ClockTime& t) { abort(); }
    void _event    (const void* payload, const Pds::ClockTime& t) { abort(); }
  protected:
    TimepixHandler(const Pds::DetInfo& info, const EntryImage*);
    EntryImage* _entry;
    unsigned    _defColumns;
    unsigned    _defRows;
  };
};

#endif
