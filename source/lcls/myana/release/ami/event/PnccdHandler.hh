#ifndef Ami_PnccdHandler_hh
#define Ami_PnccdHandler_hh

#include "ami/event/EventHandler.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"

namespace Ami {
  class EntryImage;
  class FeatureCache;
  class PixelCalibration;

  class PnccdHandler : public EventHandler {
  public:
    PnccdHandler(const Pds::DetInfo& info, const FeatureCache&);
    ~PnccdHandler();
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
    //    PnccdHandler(const Pds::DetInfo& info, const EntryImage*);
    void _begin_calib();
    void _end_calib();
    void _fillQuadrant (const uint16_t* d, unsigned x, unsigned y);
    void _fillQuadrantR(const uint16_t* d, unsigned x, unsigned y);
  private:
    const FeatureCache&  _cache;
    Pds::PNCCD::ConfigV1 _config;
    EntryImage*          _correct;
    PixelCalibration*    _calib;
    bool                 _collect;
    unsigned             _ncollect;
    EntryImage*          _entry;
    bool                 _tform;
  };
};

#endif
