#ifndef Ami_OceanOpticsHandler_hh
#define Ami_OceanOpticsHandler_hh

#include "ami/event/EventHandler.hh"

#include "pdsdata/oceanoptics/ConfigV1.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {
  class EntryWaveform;
  class EntryRef;

  class OceanOpticsHandler : public EventHandler {
  public:
    OceanOpticsHandler(const Pds::DetInfo& info);
    ~OceanOpticsHandler();
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
    OceanOpticsHandler(const Pds::DetInfo& info, 
           const Pds::OceanOptics::ConfigV1& config);
  private:
    Pds::OceanOptics::ConfigV1 _config;
    enum { MaxEntries=32 };
    unsigned       _nentries;    
    EntryWaveform* _entry[MaxEntries];
  };
};

#endif
