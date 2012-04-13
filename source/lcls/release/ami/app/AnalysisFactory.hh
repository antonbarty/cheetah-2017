#ifndef Ami_AnalysisFactory_hh
#define Ami_AnalysisFactory_hh

#include "ami/server/Factory.hh"

#include "ami/data/Cds.hh"

#include <list>

namespace Ami {

  class Analysis;
  class FeatureCache;
  class Message;
  class Server;
  class ServerManager;
  class UserModule;
  class EventFilter;

  class AnalysisFactory : public Factory {
  public:
    AnalysisFactory(FeatureCache&,
		    ServerManager&,
		    std::list<UserModule*>&,
                    EventFilter&);
    ~AnalysisFactory();
  public:
    FeatureCache& features();
    Cds& discovery();
    Cds& hidden   ();
    void discover ();
    void configure(unsigned, const Message&, const char*, Cds&);
    void analyze  ();
    void wait_for_configure();
  private:
    ServerManager& _srv;
    Cds       _cds;
    Cds       _ocds;
    typedef std::list<Analysis*>     AnList;
    AnList    _analyses;
    Semaphore _configured;
    Semaphore _sem;
    FeatureCache& _features;
    typedef std::list<UserModule*> UList;
    UList&        _user;
    EventFilter&  _filter;
  };

};

#endif
