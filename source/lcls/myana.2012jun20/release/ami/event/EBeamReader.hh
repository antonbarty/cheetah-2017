#ifndef Ami_EBeamReader_hh
#define Ami_EBeamReader_hh

#include "ami/event/EventHandler.hh"

#include "ami/data/FeatureCache.hh"

namespace Ami {

  class EBeamReader : public EventHandler {
  public:
    EBeamReader(FeatureCache&);
    ~EBeamReader();
  public:
    void   _configure(Pds::TypeId, const void* payload, const Pds::ClockTime& t);
    void   _calibrate(Pds::TypeId, const void* payload, const Pds::ClockTime& t);
    void   _event    (Pds::TypeId, const void* payload, const Pds::ClockTime& t);
    void   _damaged  ();
  protected:
    void   _calibrate(const void* payload, const Pds::ClockTime& t) {}
    void   _configure(const void* payload, const Pds::ClockTime& t) {}
    void   _event    (const void* payload, const Pds::ClockTime& t) {}
  public:
    unsigned     nentries() const;
    const Entry* entry   (unsigned) const;
    void         reset   ();
  private:
    FeatureCache& _cache;
    int           _index;
    int           _nvars;
  };

};

#endif
