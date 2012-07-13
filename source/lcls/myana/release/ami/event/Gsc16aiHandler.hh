#ifndef Ami_Gsc16aiHandler_hh
#define Ami_Gsc16aiHandler_hh

#include "ami/event/EventHandler.hh"
#include "pdsdata/gsc16ai/ConfigV1.hh"

namespace Pds {
  class DetInfo;
};

namespace Ami {
  class FeatureCache;

  class Gsc16aiHandler : public EventHandler {
  public:
    Gsc16aiHandler(const DetInfo&, FeatureCache&);
    ~Gsc16aiHandler();
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
    FeatureCache&          _cache;
    int                    _index;
    Pds::Gsc16ai::ConfigV1 _config;
    double                 _voltsMin;
    double                 _voltsPerCount;
  };
};

#endif
