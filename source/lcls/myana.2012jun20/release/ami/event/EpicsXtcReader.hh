#ifndef Ami_EpicsXtcReader_hh
#define Ami_EpicsXtcReader_hh

#include "ami/event/EventHandler.hh"

#include "ami/data/FeatureCache.hh"

namespace Pds {
  class Dgram;
};

namespace Ami {

  class EpicsXtcReader : public EventHandler {
  public:
    EpicsXtcReader(const Pds::Src&, FeatureCache&);
    ~EpicsXtcReader();
  public:
    void   _calibrate(const void* payload, const Pds::ClockTime& t);
    void   _configure(const void* payload, const Pds::ClockTime& t);
    void   _event    (const void* payload, const Pds::ClockTime& t);
    void   _damaged  ();
  public:
    unsigned     nentries() const;
    const Entry* entry   (unsigned) const;
    void         reset   ();
  private:
    FeatureCache& _cache;
    enum { MaxPvs=1024 };
    int           _index[MaxPvs];
  };

};

#endif
