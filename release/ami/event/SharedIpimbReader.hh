#ifndef Ami_SharedIpimbReader_hh
#define Ami_SharedIpimbReader_hh

#include "ami/event/EventHandler.hh"
#include "ami/data/FeatureCache.hh"
#include "pdsdata/xtc/BldInfo.hh"

namespace Ami {

  class SharedIpimbReader : public EventHandler {
  public:
    SharedIpimbReader(const Pds::BldInfo&, FeatureCache&);
    ~SharedIpimbReader();
  public:
    void   _configure(const void* payload, const Pds::ClockTime& t);
    void   _calibrate(const void* payload, const Pds::ClockTime& t);
    void   _event    (const void* payload, const Pds::ClockTime& t);
    void   _damaged  ();
  public:
    unsigned     nentries() const;
    const Entry* entry   (unsigned) const;
    void         reset   ();
  private:
    FeatureCache& _cache;
    enum { NChannels= 11 };
    int                     _index[NChannels];
  };

};

#endif
