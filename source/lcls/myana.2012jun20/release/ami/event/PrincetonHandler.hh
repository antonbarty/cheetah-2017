#ifndef Ami_PrincetonHandler_hh
#define Ami_PrincetonHandler_hh

#include "ami/event/EventHandler.hh"
#include "ami/data/FeatureCache.hh"
#include "pdsdata/princeton/ConfigV3.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {
  class EntryImage;

  class PrincetonHandler : public EventHandler {
  public:
    PrincetonHandler(const Pds::DetInfo& info, FeatureCache& cache);
    ~PrincetonHandler();
  public:
    unsigned     nentries() const;
    const Entry* entry(unsigned) const;
    void         reset();
  private:
    virtual void _configure(Pds::TypeId type, const void* payload, const Pds::ClockTime& t);    
    virtual void _calibrate(const void* payload, const Pds::ClockTime& t);
    virtual void _event    (Pds::TypeId type, const void* payload, const Pds::ClockTime& t);
    virtual void _damaged  ();
  private:
    //PrincetonHandler(const Pds::DetInfo& info, const EntryImage*);
    Pds::Princeton::ConfigV3  _config;
    FeatureCache&             _cache;
    int                       _iCacheIndexTemperature;
    EntryImage*               _entry;

    /*
     * These functions will never be called. The above _configure() and _event() replace these two.
     */    
    virtual void _configure(const void* payload, const Pds::ClockTime& t);
    virtual void _event    (const void* payload, const Pds::ClockTime& t);
  };
};

#endif
