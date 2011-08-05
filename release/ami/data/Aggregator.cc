#include "ami/data/Aggregator.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/EntryFactory.hh"
#include "ami/data/EntryScan.hh"
#include "ami/service/BSocket.hh"

#include <sys/types.h>
#include <sys/socket.h>

using namespace Ami;

static const int BufferSize = 0x800000;

template <class T>
static void agg(iovec*, iovec*, char*);

Aggregator::Aggregator(AbsClient& client) :
  _client          (client),
  _n               (0),
  _remaining       (0),
  _cds             ("Aggregator"),
  _niovload        (5),
  _iovload         (new iovec[_niovload]),
  _iovdesc         (new iovec[_niovload+1]),
  _buffer          (new BSocket(BufferSize)),
  _state           (Init),
  _latest          (0)
{
}

Aggregator::~Aggregator()
{
}

//
//  Add another server
//
void Aggregator::connected       () 
{
  //  printf("Agg connect %d %p\n",_n,&_client);
  //
  //  Only forward a connection if it is the first, or it
  //    requires a resync of server states
  //
  if (_n++==0 || _state!=Init)
    _client.connected(); 
  else
    printf("Agg conn suppressed n (%d) _state (%d)\n",
           _n, _state);
}

void Aggregator::disconnected    () 
{
  //  printf("Agg disconnect %d %p\n",_n,&_client);
  _n--;
  _client.disconnected(); 
}

//
//  Scale down the statistics required from each server
//
int  Aggregator::configure       (iovec* iov) 
{
  _state = Configured;
  int n = _client.configure(iov);
  // ...
  return n;
}

int  Aggregator::configured      () 
{
  return _client.configured(); 
}

void Aggregator::discovered      (const DiscoveryRx& rx)
{
  _client.discovered(rx); 
}

//
//  Keep a cache of the description
//
void Aggregator::read_description(Socket& socket, int len) 
{
  if (_n == 1) {
    _client.read_description(socket,len);
    _state = Described;
    _remaining = 0;
  }
  else if ( _n > 1 ) {

    if (len > BufferSize) {
      printf("Aggregator::read_description too large to buffer (%d)\n",len);
      return;
    }

    int size = socket.read(_buffer->data(),len);

    if (_state != Describing) {
      
      _cds.reset();
    
      const char* payload = _buffer->data();
      const char* const end = payload + size;
      payload += sizeof(Desc);
      
      while( payload < end ) {
	const DescEntry* desc = reinterpret_cast<const DescEntry*>(payload);
	if (desc->size()==0) {
	  printf("read_description size==0\n");
	  break;
	}
	Entry* entry = EntryFactory::entry(*desc);
	_cds.add(entry, desc->signature());
	payload += desc->size();
// 	printf("%s[%d]  norm %c  agg %c\n",
// 	       desc->name(),
// 	       desc->signature(),
// 	       desc->isnormalized() ? 't':'f',
// 	       desc->aggregate() ? 't':'f');
      }

      if (_cds.totalentries()>_niovload) {
	delete[] _iovload;
	_iovload = new iovec[_niovload=_cds.totalentries()];
	delete[] _iovdesc;
	_iovdesc = new iovec[_cds.totaldescs()];
      }
      _cds.payload    (_iovload);
      _cds.description(_iovdesc);

      _client.read_description(*_buffer,len);
      _remaining = _n-1;
      _state = Describing;
    }
    else if (--_remaining == 0) {
      _state = Described;
    }
  }
}

//
//  Add the payload from one server, and pass the results if all servers seen
//
#define CASEADD(type,f)							\
  case DescEntry::type:							\
  reinterpret_cast<Entry##type*>(payload)->f(*reinterpret_cast<Entry##type*>(iovl->iov_base)); \
  break;

#define SETNORM(type,e)							\
  case DescEntry::type:							\
  norm = reinterpret_cast<Entry##type*>(e)->info(Entry##type::Normalization); \
  break;

#include "pdsdata/xtc/ClockTime.hh"

void Aggregator::read_payload    (Socket& s, int sz) 
{
  if (_state != Described) 
    return;

  if ( _n == 1 ) { 
    _client.read_payload(s,sz);
    _remaining = 0;
  }
  else if ( _n > 1 ) {

    if (_remaining==0) {  // simply copy
      s.read(_buffer->data(),sz);
      _remaining = _n-1;
    }
    else {  // aggregate
      int niov = _cds.totalentries();
      s.readv(_iovload,niov);
      iovec* iovl = _iovload;
      iovec* iovd = _iovdesc+1;
      char* payload = _buffer->data();
      while(niov--) {
	DescEntry* desc = reinterpret_cast<DescEntry*>(iovd->iov_base);
	switch(desc->type()) {
	case DescEntry::Scalar:
	case DescEntry::TH1F:
	case DescEntry::Waveform:
	case DescEntry::Prof:
          agg<double>(iovd,iovl,payload);
	  break;
	case DescEntry::Image:
          agg<unsigned>(iovd,iovl,payload);
	  break;
	case DescEntry::Scan:  // This one's hard
// 	  printf("Agg Scan\n");
	  //  Consider scans that don't gather enough events to see all servers
	  //  Consider bld that is different for every event
	  { double* dst = reinterpret_cast<double*>(payload);
	    const double* end = reinterpret_cast<const double*>(payload+iovl->iov_len);
	    const double* src = reinterpret_cast<const double*>(iovl->iov_base);

	    EntryScan* t = new EntryScan(*reinterpret_cast<const DescScan*>(desc));
	    t->sum(dst,src);

	    src = reinterpret_cast<const double*>(t->payload());
	    while(dst < end)
	      *dst++ = *src++;

	    delete t;
	  }	  
	  break;
	case DescEntry::TH2F:
	default:
// 	  printf("Agg Other\n");
	  break;
	}
	payload += iovl->iov_len;
	iovl++;
	iovd++;
      }
      if (--_remaining == 0) {
	_client.read_payload(*_buffer,sz);
      }
    }
  }
}

//
//  If all servers seen, process.
//
void Aggregator::process         () 
{
  if (_state==Described && _remaining==0)
    _client.process();
}

template <class T>
void agg(iovec* iovd, iovec* iovl, char* payload)
{
  DescEntry* desc = reinterpret_cast<DescEntry*>(iovd->iov_base);
  const char* base = (const char*)iovl->iov_base;
  //  the first word is the valid flag (update time)
  T* dst = reinterpret_cast<T*>(payload);
  const T* src = reinterpret_cast<const T*>(base);
  const T* end = reinterpret_cast<const T*>(base+iovl->iov_len);
#if 0
  { const Pds::ClockTime* dst_clk = reinterpret_cast<const Pds::ClockTime*>(dst);
    const Pds::ClockTime* src_clk = reinterpret_cast<const Pds::ClockTime*>(src);
    printf("Agg Std agg %c  dst %09u.%09u  src %09u.%09u\n",
           desc->aggregate() ? 't':'f', 
           dst_clk->seconds(), dst_clk->nanoseconds(),
           src_clk->seconds(), src_clk->nanoseconds()); }
#endif
  if (desc->aggregate() && *dst!=0 && *src!=0 ) { // sum them 
    if (*dst < *src)
      *dst = *src;   // record later time
    while (++src < end)
      *++dst += *src;  
  }
  else if (*dst < *src)  // copy most recent (including time)
    while (src < end)
      *dst++  = *src++;  
} 
