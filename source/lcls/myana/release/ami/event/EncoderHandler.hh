#ifndef Ami_EncoderReader_hh
#define Ami_EncoderReader_hh

#include "ami/event/EventHandler.hh"

namespace Pds {
  class DetInfo;
};

namespace Ami {
  class FeatureCache;

  class EncoderHandler : public EventHandler {
  public:
    EncoderHandler(const DetInfo&, FeatureCache&);
    ~EncoderHandler();
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
  };

};

#endif
