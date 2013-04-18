#include "SplitEventQ.hh"

#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/xtc/XtcIterator.hh"

using std::list;
using namespace Pds;

//#define DBUG

static const unsigned MaxSize = 0x2000000;

class Locator : public XtcIterator {
public:
  Locator(Dgram* frag) : _frag(frag) {}
  ~Locator() {}
public:
  const Xtc* locate(const Xtc* xtc) {
    _request  = xtc;
    _match    = 0;
    iterate(&_frag->xtc);
    return _match;
  }
  int process(Xtc* xtc) {
    if (xtc->src == _request->src) {
      _match = xtc;
      return 0;
    }
    else if (xtc->contains.id()==TypeId::Id_Xtc) {
      iterate(xtc);
      return _match ? 0 : 1;
    }
    return 1;
  }
private:
  Dgram*     _frag;
  const Xtc* _request;
  const Xtc* _match;
};

class Merger : public XtcIterator {
public:
  Merger(Dgram* frag) : _locator(frag) {}
  ~Merger() {}
public:
  bool merge(Dgram* dg)
  {
    _missed = false;
    do {
      _endp   = reinterpret_cast<char*>(dg->xtc.next());
      _extent = 0;
      iterate(&dg->xtc);
      dg->xtc.extent += _extent;
    } while(_extent);
    return !_missed;
  }
  int process(Xtc* xtc)
  {
    if (xtc->contains.id()==TypeId::Any &&
        xtc->damage.value()==(1<<Damage::DroppedContribution)) {
      const Xtc* rxtc = _locator.locate(xtc);
      if (rxtc) {
        _extent = rxtc->extent-xtc->extent;
        char* p = reinterpret_cast<char*>(xtc);
        char* q = reinterpret_cast<char*>(xtc->next());
        memcpy(p+rxtc->extent,q,_endp-q);
        memcpy(p,reinterpret_cast<const char*>(rxtc),rxtc->extent);
        return 0;
      }
      else
        _missed=true;
    }
    else if (xtc->contains.id()==TypeId::Id_Xtc) {
      iterate(xtc);
      if (_extent) {
        xtc->extent += _extent;
        return 0;
      }
    }
    return 1;
  }
private:
  Locator  _locator;
  char*    _endp;
  unsigned _extent;
  bool     _missed;
};


SplitEventQ::SplitEventQ(unsigned depth) :
  _depth(depth),
  _completed(0),
  _cached(0),
  _aborted(0)
{
}

SplitEventQ::~SplitEventQ()
{
  clear();
}

bool SplitEventQ::cache(Pds::Dgram* dg)
{
  if (_depth==0 || (dg->xtc.damage.value()&(1<<Damage::DroppedContribution))==0)
    return false;

  list<Dgram*>::iterator it=_q.begin();
  while(it!=_q.end()) {
    const ClockTime& clk = (*it)->seq.clock();
    if (clk > dg->seq.clock()) 
      break;
    else if(dg->seq.clock() == clk) {
      Merger m(dg);
      if (m.merge(*it)) {
        _completed++;
#ifdef DBUG
        printf("Cmpletd event %08x.%08x %04x\n",
               dg->seq.clock().seconds(),
               dg->seq.clock().nanoseconds(),
               dg->seq.stamp().fiducials());
#endif
        delete[] reinterpret_cast<char*>(*it);
        _q.erase(it);
        return false;
      }
      else {
#ifdef DBUG
        printf("Merged  event %08x.%08x %04x\n",
               dg->seq.clock().seconds(),
               dg->seq.clock().nanoseconds(),
               dg->seq.stamp().fiducials());
#endif
        memcpy(*it,dg,dg->xtc.sizeofPayload()+sizeof(*dg));
        return true;
      }
    }
    it++;
  }

#ifdef DBUG
  printf("Caching event %08x.%08x %04x\n",
         dg->seq.clock().seconds(),
         dg->seq.clock().nanoseconds(),
         dg->seq.stamp().fiducials());
#endif
  _cached++;
  if (_q.size() < _depth) {
    char* p = new char[MaxSize];
    memcpy(p,dg,dg->xtc.sizeofPayload()+sizeof(*dg));
    _q.insert(it,reinterpret_cast<Dgram*>(p));
    return true;
  }
  else {
    char* p = new char[MaxSize];
    memcpy(p,dg,dg->xtc.sizeofPayload()+sizeof(*dg));
    _q.insert(it,reinterpret_cast<Dgram*>(p));

    _aborted++;
    Dgram* odg = _q.front();
#ifdef DBUG
    printf("Aborted event %08x.%08x %04x\n",
           odg->seq.clock().seconds(),
           odg->seq.clock().nanoseconds(),
           odg->seq.stamp().fiducials());
#endif
    _q.pop_front();
    memcpy(dg,odg,odg->xtc.sizeofPayload()+sizeof(*odg));
    delete[] reinterpret_cast<char*>(odg);
    return false;
  }
}

list<Dgram*>& SplitEventQ::queue() { return _q; }

void SplitEventQ::clear()
{
  if (_depth && _cached)
    printf("Recovered %d/%d split events, aborted %d\n", _completed, _cached, _aborted);
  _completed = _cached = _aborted = 0;
  for(list<Dgram*>::iterator it=_q.begin(); it!=_q.end(); it++)
    delete[] reinterpret_cast<char*>(*it);
  _q.clear();
}
