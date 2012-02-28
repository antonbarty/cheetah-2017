#ifndef Ami_EventFilter_hh
#define Ami_EventFilter_hh

#include "pdsdata/xtc/XtcIterator.hh"

#include "ami/data/UserModule.hh"
#include <list>
#include <vector>

namespace Pds { class Dgram; class Xtc; class Sequence; };

namespace Ami {
  class FeatureCache;

  class EventFilter : private Pds::XtcIterator {
  public:
    EventFilter(std::list<UserModule*>& filters,
                FeatureCache&           cache);
    ~EventFilter();
  public:
    void reset       ();
    void enable      (unsigned);
    void configure   (Dgram*);
    bool accept      (Dgram*);
  private:
    int process(Xtc*);
  private:
    std::list<UserModule*>& _filters;
    FeatureCache&           _cache;
    unsigned                _enable;
    const Pds::Sequence*    _seq;
  };
};

#endif
