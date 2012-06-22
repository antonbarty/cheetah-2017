#ifndef Ami_FrameFexHandler_hh
#define Ami_FrameFexHandler_hh

#include "ami/event/EventHandler.hh"

namespace Pds {
  class DetInfo;
};

namespace Ami {
  class FeatureCache;

  class FrameFexHandler : public EventHandler {
  public:
    FrameFexHandler(const DetInfo&, FeatureCache&);
    ~FrameFexHandler();
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
    FeatureCache&        _cache;
    enum { Integral, XMean, YMean, Major, Minor, Tilt, NChannels=6 };
    int                  _index[NChannels];
  };

};

#endif
