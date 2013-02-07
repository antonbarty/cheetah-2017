#ifndef Ami_XtcClient_hh
#define Ami_XtcClient_hh

#include "pdsdata/xtc/ClockTime.hh"
#include "pdsdata/xtc/XtcIterator.hh"

#include "ami/app/EventFilter.hh"

#include <list>

namespace Pds {
  class Dgram;
  class Xtc;
  class Sequence;
};

namespace Ami {

  class FeatureCache;
  class Factory;
  class Composer;
  class EventHandler;
  class EventFilter;
  class Entry;
  class UserModule;

  class XtcClient : private XtcIterator {
  public:
    XtcClient(std::vector<FeatureCache*>& cache, 
	      Factory&      factory, 
	      std::list<UserModule*>& user_ana,
	      EventFilter&  filter,
	      bool          sync=false);
    ~XtcClient();
    void insert(EventHandler*);
    void remove(EventHandler*);
    void processDgram(Pds::Dgram*);
  private:
    typedef std::list<EventHandler*> HList;
    typedef std::list<Composer*>     CList;
    typedef std::list<UserModule*>   UList;
    typedef std::list<Entry*>        EList;
    int process(Pds::Xtc*);
    void _configure(Pds::Xtc* xtc, EventHandler* h);
    std::vector<FeatureCache*>& _cache;
    Factory&      _factory;
    UList&        _user_ana;
    EventFilter&  _filter;
    const Pds::Sequence* _seq;
    bool      _sync;
    HList     _handlers;
    CList     _composers;
    EList     _entry;
    bool      _ready;
    int       _ptime_index, _ptime_acc_index;    
    int       _pltnc_index;
    int       _event_index;
  };
}

#endif
