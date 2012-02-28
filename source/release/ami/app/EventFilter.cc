#include "ami/app/EventFilter.hh"

#include "ami/data/FeatureCache.hh"

#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/xtc/Xtc.hh"

#include <string>
using std::string;
using std::list;
using namespace Ami;

EventFilter::EventFilter(list<UserModule*>& filters,
                         FeatureCache& cache) :
  _filters(filters),
  _cache  (cache),
  _enable (0)
{
}

EventFilter::~EventFilter()
{
  for(list<UserModule*>::iterator it=_filters.begin(); 
      it!=_filters.end(); it++)
    delete (*it);

}

void EventFilter::enable (unsigned o)
{
  _enable = o;
}

void EventFilter::reset  ()
{
  for(list<UserModule*>::iterator it=_filters.begin(); it!=_filters.end(); it++)
    (*it)->reset(_cache);
}

void EventFilter::configure   (Dgram* dg)
{
  for(list<Ami::UserModule*>::iterator it=_filters.begin();
      it!=_filters.end(); it++)
    (*it)-> clock(dg->seq.clock());

  _seq = &dg->seq;
  iterate(&dg->xtc);
}

bool Ami::EventFilter::accept(Dgram* dg)
{
  if (_filters.empty())
    return true;

  for(list<Ami::UserModule*>::iterator it=_filters.begin();
      it!=_filters.end(); it++)
    (*it)-> clock(dg->seq.clock());

  _seq = &dg->seq;
  iterate(&dg->xtc);

  bool result=true;
  int i=0;
  for(list<Ami::UserModule*>::iterator it=_filters.begin();
      it!=_filters.end(); it++) {
    if (!(*it)->accept())
      result = result && ((_enable&(1<<i))==0);
  }
  return result;
}

int Ami::EventFilter::process(Xtc* xtc)
{
  if (xtc->extent < sizeof(Xtc) ||
      xtc->contains.id() >= TypeId::NumberOf)
    return 0;

  if (xtc->contains.id()==TypeId::Id_Xtc)
    iterate(xtc);
  else if (_seq->service()==TransitionId::L1Accept) {
    if (xtc->damage.value()==0)
      for(list<Ami::UserModule*>::iterator it=_filters.begin();
          it!=_filters.end(); it++)
        (*it)-> event(xtc->src,
                      xtc->contains,
                      xtc->payload());
  }
  else if (_seq->service()==TransitionId::Configure) {
    for(list<Ami::UserModule*>::iterator it=_filters.begin();
        it!=_filters.end(); it++)
      if (xtc->damage.value()==0)
        (*it)-> configure(xtc->src,
                          xtc->contains,
                          xtc->payload());
  }
  else
    ;
  return 1;
}
