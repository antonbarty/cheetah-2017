#ifndef Ami_AcqWaveformHandler_hh
#define Ami_AcqWaveformHandler_hh

#include "ami/event/EventHandler.hh"

#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {
  class EntryWaveform;
  class EntryRef;

  class AcqWaveformHandler : public EventHandler {
  public:
    AcqWaveformHandler(const Pds::DetInfo& info);
    ~AcqWaveformHandler();
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
    AcqWaveformHandler(const Pds::DetInfo& info, 
		       const Pds::Acqiris::ConfigV1& config);
  private:
    Pds::Acqiris::ConfigV1 _config;
    enum { MaxEntries=32 };
    unsigned       _nentries;    
    EntryWaveform* _entry[MaxEntries];
    EntryRef*      _ref;
  };
};

#endif
