#include "AnalysisFactory.hh"

#include "ami/app/SummaryAnalysis.hh"
#include "ami/app/EventFilter.hh"
#include "ami/data/Analysis.hh"
#include "ami/data/UserModule.hh"

#include "ami/data/FeatureCache.hh"
#include "ami/data/Message.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/server/ServerManager.hh"

using namespace Ami;


AnalysisFactory::AnalysisFactory(FeatureCache&  cache,
				 ServerManager& srv,
                                 UList&         user,
                                 EventFilter&   filter) :
  _srv       (srv),
  _cds       ("Analysis"),
  _ocds      ("Hidden"),
  _configured(Semaphore::EMPTY),
  _sem       (Semaphore::FULL),
  _features  (cache),
  _user      (user),
  _filter    (filter)
{
}

AnalysisFactory::~AnalysisFactory()
{
}

FeatureCache& AnalysisFactory::features() { return _features; }

Cds& AnalysisFactory::discovery() { return _cds; }
Cds& AnalysisFactory::hidden   () { return _ocds; }

//
//  The discovery cds has changed
//
void AnalysisFactory::discover () 
{
  _sem.take();
  for(AnList::iterator it=_analyses.begin(); it!=_analyses.end(); it++) {
    Analysis* an = *it;
    delete an;
  }
  _analyses.clear();
  _sem.give();
  _srv.discover(); 
  printf("AnalysisFactory::discover complete\n");
}

void AnalysisFactory::configure(unsigned       id,
				const Message& msg, 
				const char*    payload, 
				Cds&           cds)
{
  printf("AnalysisFactory::configure\n");

  _sem.take();
  AnList newlist;
  for(AnList::iterator it=_analyses.begin(); it!=_analyses.end(); it++) {
    Analysis* a = *it;
    if (a->id() == id) {
      delete a;
    }
    else
      newlist.push_back(a);
  }
  _analyses = newlist;

  // create
  const char* const end = payload + msg.payload();
  while(payload < end) {
    const ConfigureRequest& req = *reinterpret_cast<const ConfigureRequest*>(payload);

    if (req.source() == ConfigureRequest::Summary) {
      SummaryAnalysis::instance().clear();
      SummaryAnalysis::instance().create(cds);
    }
    else if (req.source() == ConfigureRequest::User) {
      for(UList::iterator it=_user.begin(); it!=_user.end(); it++) {
        (*it)->clear();
        (*it)->create(cds);
      }
    }
    else if (req.source() == ConfigureRequest::Filter) {
      unsigned o = *reinterpret_cast<const uint32_t*>(&req+1);
      _filter.enable(o);
    }
    else {
      const Cds* pcds = 0;
      switch(req.source()) {
      case ConfigureRequest::Discovery: pcds = &_cds; break;
      case ConfigureRequest::Analysis : pcds = & cds; break;
      case ConfigureRequest::Hidden   : pcds = &_ocds; break;
      default:
        printf("AnalysisFactory::configure unknown source %d\n",req.source());
        break;
      }
      const Cds& input_cds = *pcds;
      const Entry* input = input_cds.entry(req.input());
      if (!input) {
	printf("AnalysisFactory::configure failed input for configure request:\n");
	printf("\tinp %d  out %d  size %d\n",req.input(),req.output(),req.size());
        input_cds.showentries();
      }
      else if (req.state()==ConfigureRequest::Create) {
	const char*  p     = reinterpret_cast<const char*>(&req+1);
	Analysis* a = new Analysis(id, *input, req.output(),
				   cds, _features, p);
	_analyses.push_back(a);
      }
      else if (req.state()==ConfigureRequest::SetOpt) {
        unsigned o = *reinterpret_cast<const uint32_t*>(&req+1);
        printf("Set options %x on entry %p\n",o,input);
        const_cast<Ami::DescEntry&>(input->desc()).options(o);
      }
    }
    payload += req.size();
  }
  _sem.give();

  _configured.give();
}

void AnalysisFactory::analyze  ()
{
  _sem.take();
  SummaryAnalysis::instance().analyze();
  for(AnList::iterator it=_analyses.begin(); it!=_analyses.end(); it++) {
    (*it)->analyze();
  }
  _sem.give();
}

void AnalysisFactory::wait_for_configure() { _configured.take(); }
