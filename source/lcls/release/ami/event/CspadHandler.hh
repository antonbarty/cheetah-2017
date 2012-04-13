#ifndef Ami_CspadHandler_hh
#define Ami_CspadHandler_hh

#include "ami/event/EventHandler.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pds/config/CsPadConfigType.hh"

namespace CspadGeometry { 
  class Detector; 
  class ConfigCache;
};

namespace Ami {
  class EntryImage;
  class FeatureCache;

  class CspadHandler : public EventHandler {
  public:
    CspadHandler(const Pds::DetInfo& info, FeatureCache&, 
                 unsigned max_pixels=600);
    ~CspadHandler();
  public:
    unsigned     nentries() const;
    const Entry* entry(unsigned) const;
    const Entry* hidden_entry(unsigned) const;
    void         reset();
  protected:
    void _calibrate(const void* payload, const Pds::ClockTime& t);
    void _configure(Pds::TypeId, const void* payload, const Pds::ClockTime& t);
    void _configure(const void* payload, const Pds::ClockTime& t) {}
    void _calibrate(Pds::TypeId::Type, const void* payload, const Pds::ClockTime& t);
    void _event    (const void* payload, const Pds::ClockTime& t);
    void _damaged  ();
  private:
    void _create_entry(const CspadGeometry::ConfigCache& cfg,
                       FILE* f, FILE* s, FILE* g, FILE* gm,
                       CspadGeometry::Detector*& detector,
                       EntryImage*& entry, 
                       unsigned max_pixels); 
  protected:
    //    CspadHandler(const Pds::DetInfo& info, const EntryImage*);
    EntryImage* _entry;
    EntryImage* _unbinned_entry;
    CspadGeometry::Detector* _detector;
    CspadGeometry::Detector* _unbinned_detector;
    FeatureCache&        _cache;
    unsigned             _max_pixels;
    unsigned             _options;
  };
};

#endif
