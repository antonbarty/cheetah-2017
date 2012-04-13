#include "XtcClient.hh"

#include "ami/app/SummaryAnalysis.hh"
#include "ami/app/EventFilter.hh"

#include "ami/event/EventHandler.hh"
#include "ami/event/EvrHandler.hh"
#include "ami/event/FEEGasDetEnergyReader.hh"
#include "ami/event/EBeamReader.hh"
#include "ami/event/PhaseCavityReader.hh"
#include "ami/event/EpicsXtcReader.hh"
#include "ami/event/SharedIpimbReader.hh"
#include "ami/event/SharedPimHandler.hh"
#include "ami/event/ControlXtcReader.hh"
#include "ami/event/IpimbHandler.hh"
#include "ami/event/EncoderHandler.hh"
#include "ami/event/Gsc16aiHandler.hh"
#include "ami/event/Opal1kHandler.hh"
#include "ami/event/TM6740Handler.hh"
#include "ami/event/FccdHandler.hh"
#include "ami/event/PnccdHandler.hh"
#include "ami/event/CspadHandler.hh"
#include "ami/event/CspadMiniHandler.hh"
#include "ami/event/PrincetonHandler.hh"
#include "ami/event/AcqWaveformHandler.hh"
#include "ami/event/AcqTdcHandler.hh"
#include "ami/event/DiodeFexHandler.hh"
#include "ami/event/IpmFexHandler.hh"
#include "ami/data/FeatureCache.hh"
#include "ami/data/Cds.hh"
#include "ami/data/EntryScalar.hh"
#include "ami/data/UserModule.hh"
#include "ami/server/Factory.hh"

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Dgram.hh"

using namespace Ami;

XtcClient::XtcClient(FeatureCache& cache, 
		     Factory&      factory,
		     UList&        user_ana,
                     EventFilter&  filter,
		     bool          sync) :
  _cache   (cache),
  _factory (factory),
  _user_ana(user_ana),
  _filter  (filter),
  _sync    (sync),
  _ready   (false),
  _ptime_index    (-1),
  _ptime_acc_index(-1),
  _pltnc_index    (-1),
  _event_index    (-1)
{
}

XtcClient::~XtcClient()
{
}

void XtcClient::insert(EventHandler* h) { _handlers.push_back(h); }
void XtcClient::remove(EventHandler* h) { _handlers.remove(h); }

void XtcClient::processDgram(Pds::Dgram* dg) 
{
  bool accept=false;

  timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);

  //  if (dg->seq.isEvent() && dg->xtc.damage.value()==0) {
  if (dg->seq.isEvent() && _ready) {
    _seq = &dg->seq;

    if (_filter.accept(dg)) {
      accept = true;

      _cache.cache(_event_index,_seq->stamp().vector());
      SummaryAnalysis::instance().clock(dg->seq.clock());
      for(UList::iterator it=_user_ana.begin(); it!=_user_ana.end(); it++)
        (*it)->clock(dg->seq.clock());
      
      iterate(&dg->xtc); 
      
      _entry.front()->valid(_seq->clock());
      _factory.analyze();
    }
  }
  else if (dg->seq.service() == Pds::TransitionId::Configure) {

    printf("XtcClient configure\n");

    _cache.clear();

    //  Cleanup previous entries
    _factory.discovery().reset();
    _factory.hidden   ().reset();
    SummaryAnalysis::instance().reset();
    for(HList::iterator it = _handlers.begin(); it != _handlers.end(); it++)
      (*it)->reset();

    _filter.reset();
    _filter.configure(dg);

    _seq = &dg->seq;
    SummaryAnalysis::instance().clock(dg->seq.clock());
    for(UList::iterator it=_user_ana.begin(); it!=_user_ana.end(); it++)
      (*it)->clock(dg->seq.clock());
    
    iterate(&dg->xtc); 

    //  Create and register new entries
    _entry.clear();
    { ProcInfo info(Pds::Level::Control,0,0);
      EntryScalar* e = new EntryScalar(reinterpret_cast<const DetInfo&>(info),0,"XtcClient","timestamp");
      _factory.discovery().add(e);
      _entry.push_back(e);
      for(UList::iterator it=_user_ana.begin(); it!=_user_ana.end(); it++) {
        info = ProcInfo(Pds::Level::Event,_entry.size(),0);
        e = new EntryScalar(reinterpret_cast<const DetInfo&>(info),_entry.size(),(*it)->name(),"module");
        _factory.discovery().add(e);
        _entry.push_back(e);
      }
    }

    for(HList::iterator it = _handlers.begin(); it != _handlers.end(); it++) {
      for(unsigned k=0; k<(*it)->nentries(); k++) {                     
        const Entry* e = (*it)->entry(k);
        if (e) {
          unsigned signature = _factory.discovery().add   (const_cast<Entry*>(e));
          const Entry* o = (*it)->hidden_entry(k);
          if (o) 
            _factory.hidden().add   (const_cast<Entry*>(o),signature);
        }
      }
    }
    _factory.discovery().showentries();
    _factory.hidden   ().showentries();

    _ptime_index     = _cache.add("ProcTime");
    _ptime_acc_index = _cache.add("ProcTimeAcc");
    _pltnc_index     = _cache.add("ProcLatency");
    _event_index     = _cache.add("EventId");

    printf("XtcClient configure done\n");

    //  Advertise
    _factory.discover();
    if (_sync) _factory.wait_for_configure();

    _ready =  true;
  }
  else {
    if (dg->seq.service() == Pds::TransitionId::Unconfigure)
      _ready = false;

    _seq = &dg->seq;
    iterate(&dg->xtc); 
  }
  timespec tq;
  clock_gettime(CLOCK_REALTIME, &tq);
  if (_ptime_index>=0) {
    double dt;
    dt = double(tq.tv_sec-tp.tv_sec) + 
      1.e-9*(double(tq.tv_nsec)-double(tp.tv_nsec));
    _cache.cache(_ptime_index,dt);
    if (accept)
      _cache.cache(_ptime_acc_index,dt);

    dt = double(tq.tv_sec)-double(dg->seq.clock().seconds()) + 
      1.e-9*(double(tq.tv_nsec)-double(dg->seq.clock().nanoseconds()));
    _cache.cache(_pltnc_index,dt);
  }
}

int XtcClient::process(Pds::Xtc* xtc) 
{
  if (xtc->extent < sizeof(Xtc) ||
      xtc->contains.id() >= TypeId::NumberOf)
    return 0;

  if (xtc->contains.id() == Pds::TypeId::Id_Xtc) {
    iterate(xtc);
  }
  else {
    if (_seq->service()==Pds::TransitionId::L1Accept) {
      SummaryAnalysis::instance().event    (xtc->src,
					    xtc->contains,
					    xtc->payload());
    }
    else if (_seq->service()==Pds::TransitionId::Configure) {
      SummaryAnalysis::instance().configure(xtc->src,
					    xtc->contains,
					    xtc->payload());
    }
    for(HList::iterator it = _handlers.begin(); it != _handlers.end(); it++) {
      EventHandler* h = *it;
      if (h->info().level() == xtc->src.level() &&
	  (h->info().phy  () == (uint32_t)-1 ||
	   h->info().phy  () == xtc->src.phy())) {
	if (_seq->isEvent() && xtc->contains.id()==h->data_type()) {
	  if (xtc->damage.value())
	    h->_damaged();
	  else
	    h->_event(xtc->contains,xtc->payload(),_seq->clock());
	  return 1;
	}
	else {
	  const std::list<Pds::TypeId::Type>& types = h->config_types();
	  Pds::TypeId::Type type = xtc->contains.id();
	  for(std::list<Pds::TypeId::Type>::const_iterator it=types.begin();
	      it != types.end(); it++) {
	    if (*it == type) {
	      if (_seq->service()==Pds::TransitionId::Configure)
		h->_configure(xtc->contains, xtc->payload(), _seq->clock());
	      else if (_seq->service()==Pds::TransitionId::BeginCalibCycle)
		h->_calibrate(xtc->contains, xtc->payload(), _seq->clock());
	      else
		continue;
	      return 1;
	    }
	  }
	}
      }
    }
    //  Wasn't handled
    if (_seq->service()==Pds::TransitionId::Configure) {
      const DetInfo& info    = reinterpret_cast<const DetInfo&>(xtc->src);
      const BldInfo& bldInfo = reinterpret_cast<const BldInfo&>(xtc->src);
      EventHandler* h = 0;
      switch(xtc->contains.id()) {
      case Pds::TypeId::Id_AcqConfig:        h = new AcqWaveformHandler(info); break;
      case Pds::TypeId::Id_AcqTdcConfig:     h = new AcqTdcHandler     (info); break;
      case Pds::TypeId::Id_TM6740Config:     h = new TM6740Handler     (info); break;
      case Pds::TypeId::Id_Opal1kConfig:     h = new Opal1kHandler     (info); break;
      case Pds::TypeId::Id_FccdConfig  :     h = new FccdHandler       (info); break;
      case Pds::TypeId::Id_PrincetonConfig:  h = new PrincetonHandler  (info); break;
      case Pds::TypeId::Id_pnCCDconfig:      h = new PnccdHandler    (info,_cache); break;
      case Pds::TypeId::Id_CspadConfig:      
        if (info.device()==DetInfo::Cspad)   h = new CspadHandler    (info,_cache);
        else                                 h = new CspadMiniHandler(info,_cache);
        break;
      case Pds::TypeId::Id_ControlConfig:    h = new ControlXtcReader     (_cache); break;
      case Pds::TypeId::Id_Epics:            h = new EpicsXtcReader  (info,_cache); break;
      case Pds::TypeId::Id_FEEGasDetEnergy:  h = new FEEGasDetEnergyReader(_cache); break;
      case Pds::TypeId::Id_EBeam:            h = new EBeamReader          (_cache); break;
      case Pds::TypeId::Id_PhaseCavity:      h = new PhaseCavityReader    (_cache); break;
      case Pds::TypeId::Id_IpimbConfig:      h = new IpimbHandler    (info,_cache); break;
      case Pds::TypeId::Id_EncoderConfig:    h = new EncoderHandler  (info,_cache); break;
      case Pds::TypeId::Id_Gsc16aiConfig:    h = new Gsc16aiHandler  (info,_cache); break;
      case Pds::TypeId::Id_EvrConfig:        h = new EvrHandler      (info,_cache); break;
      case Pds::TypeId::Id_DiodeFexConfig:   h = new DiodeFexHandler (info,_cache); break;
      case Pds::TypeId::Id_IpmFexConfig:     h = new IpmFexHandler   (info,_cache); break;
      case Pds::TypeId::Id_SharedIpimb:      h = new SharedIpimbReader(bldInfo,_cache); break;
      case Pds::TypeId::Id_SharedPim:        h = new SharedPimHandler     (bldInfo); break;
      default: break;
      }
      if (!h) {
        if (xtc->contains.id()==Pds::TypeId::Id_TM6740Config)
          ;
        else
          printf("XtcClient::process cant handle type %d\n",xtc->contains.id());
      }
      else {
	printf("XtcClient::process adding handler for info %s type %s\n",
	       Pds::DetInfo::name(info), Pds::TypeId::name(xtc->contains.id()));
	insert(h);
	h->_configure(xtc->contains,xtc->payload(),_seq->clock());
      }
    }
  }
  return 1;
}
