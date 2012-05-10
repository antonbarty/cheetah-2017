#ifndef Ami_FrameHandler_hh
#define Ami_FrameHandler_hh

#include "ami/event/EventHandler.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {
  class EntryImage;

  class FrameHandler : public EventHandler {
  public:
    FrameHandler(const Pds::DetInfo& info,
		 unsigned defColumns,
		 unsigned defRows);
    FrameHandler(const Pds::DetInfo& info,
		 const std::list<Pds::TypeId::Type>& config_types,
		 unsigned defColumns,
		 unsigned defRows);
    ~FrameHandler();
  public:
    unsigned     nentries() const;
    const Entry* entry(unsigned) const;
    void         reset();
  protected:
    void _calibrate(const void* payload, const Pds::ClockTime& t);
    void _configure(const void* payload, const Pds::ClockTime& t);
    void _event    (const void* payload, const Pds::ClockTime& t);
    void _damaged  ();
  protected:
    FrameHandler(const Pds::DetInfo& info, const EntryImage*);
    EntryImage* _entry;
    unsigned    _defColumns;
    unsigned    _defRows;
  };
};

#endif
