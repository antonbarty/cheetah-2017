#include "ami/python/Client.hh"

#include "ami/client/ClientManager.hh"

#include "ami/data/ConfigureRequest.hh"
#include "ami/data/Discovery.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/service/Socket.hh"

#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>

//#define DBUG

using namespace Ami::Python;

static const int BufferSize = 0x8000;

static const Pds::DetInfo envInfo(0,
				  Pds::DetInfo::NoDetector,0,
				  Pds::DetInfo::Evr,0);

//
//  Need all info to identify source and configure plot, filter
//
Client::Client(const Pds::DetInfo& info, 
	       unsigned            channel,
	       AbsFilter*          filter,
	       AbsOperator*        op) :
  _info            (info),
  _channel         (channel),
  _filter          (filter),
  _op              (op),
  _input           (0),
  _output_signature(0),
  _request         (new char[BufferSize]),
  _description     (new char[BufferSize]),
  _cds             ("Client"),
  _manager         (0),
  _niovload        (5),
  _iovload         (new iovec[_niovload])
{
  sem_init(&_initial_sem, 0, 0);
  sem_init(&_payload_sem, 0, 0);
}

Client::~Client() 
{
  sem_destroy(&_initial_sem);
  sem_destroy(&_payload_sem);

  if (_manager) delete _manager;
  delete[] _iovload;
  delete[] _description;
  delete[] _request;
  delete   _filter;
  delete   _op;
}

void Client::connected()
{
  if (_manager) {
    _manager->discover();
  }
}

void Client::discovered(const DiscoveryRx& rx)
{
  //
  //  Find the appropriate source entry
  //    and filter inputs
  //
  const DescEntry* e = rx.entries();

#ifdef DBUG
  printf("Client Discovered info(%08x/%08x), env(%08x/%08x), cds(%d)\n",
	 _info.log(),_info.phy(), envInfo.log(),envInfo.phy(), e->signature());
#endif

  if (_info == envInfo) {
    _input = e->signature();
  }
  else {
    for(  const DescEntry* e = rx.entries(); e < rx.end(); 
	  e = reinterpret_cast<const DescEntry*>
	    (reinterpret_cast<const char*>(e) + e->size())) {

#ifdef DBUG
      printf("Client Discovered info(%08x/%08x), channel(%d)\n",
             e->info().log(),e->info().phy(), e->channel());
#endif

      if (e->info().level()==_info.level() &&
          e->info().phy  ()==_info.phy() &&
	  e->channel     ()==_channel) {
	_input = e->signature();
	break;
      }
    }
  }

#ifdef DBUG
  printf("Client Discovered (%d)\n",_input);
#endif

  _manager->configure();
}

int  Client::configure       (iovec* iov) 
{
  char* p = _request;

#ifdef DBUG
  printf("pyami Configuring filter(%p), op(%p)\n",_filter,_op);
#endif

  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  ConfigureRequest::Discovery,
						  _input,
						  ++_output_signature,
						  *_filter, *_op);

#ifdef DBUG
  printf("ConfigureRequest\n");
  const unsigned* d = reinterpret_cast<const unsigned*>(_request);
  for(unsigned i=0; i<r.size()>>2; i++)
    printf("%08x%c",d[i],(i%8)==7 ? '\n':' ');
  printf("\n");
#endif
  
  p += r.size();

  if (p > _request+BufferSize) {
    printf("Client request overflow: size = 0x%lx\n", p-_request);
    return 0;
  }
  else {
    iov[0].iov_base = _request;
    iov[0].iov_len  = p - _request;
    return 1;
  }
}

int  Client::configured      () 
{
  return 0; 
}

void Client::read_description(Socket& socket,int len)
{
  int size = socket.read(_description,len);

  if (size<0) {
    printf("Read error in Ami::Python::Client::read_description.\n");
    return;
  }

  if (size==BufferSize) {
    printf("Buffer overflow in Ami::Python::Client::read_description.  Dying...\n");
    abort();
  }

  _cds.reset();

  const char* payload = _description;
  const char* const end = payload + size;
  //  dump(payload, size);

  //  { const Desc* e = reinterpret_cast<const Desc*>(payload);
  //    printf("Cds %s\n",e->name()); }
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
//     printf("Received desc %s signature %d\n",desc->name(),desc->signature());
  }

  if (_cds.totalentries()>_niovload) {
    delete[] _iovload;
    _iovload = new iovec[_niovload=_cds.totalentries()];
  }
  _cds.payload(_iovload);

  sem_post(&_initial_sem);
}

void Client::read_payload     (Socket& socket,int)
{
  socket.readv(_iovload,_cds.totalentries());
}

void Client::process         () 
{
  sem_post(&_payload_sem);
}

void Client::managed(ClientManager& mgr)
{
  _manager = &mgr;
  _manager->connect();
}

int  Client::initialize(ClientManager& mgr)
{
  managed(mgr);

#if 1
  timespec tmo;
  clock_gettime(CLOCK_REALTIME, &tmo);

  timespec btmo;
  btmo.tv_sec  = tmo.tv_sec;
  btmo.tv_nsec = tmo.tv_nsec;

  tmo.tv_sec += 5;
  int result = sem_timedwait(&_initial_sem, &tmo);

  clock_gettime(CLOCK_REALTIME, &tmo);
  printf("Ami::Client initialize time %f seconds\n",
         double(tmo.tv_sec-btmo.tv_sec) +
         (double(tmo.tv_nsec)-double(btmo.tv_nsec))*1.e-9);
#else
  timespec tmo;
  clock_gettime(CLOCK_REALTIME, &tmo);
  tmo.tv_sec+=5;
  int result = sem_timedwait(&_initial_sem, &tmo);
#endif

  if (result<0)
    return TimedOut;
  if (_input==0)
    return NoEntry;

  return Success;
}

int Client::request_payload()
{
  _manager->request_payload();

  timespec tmo;
  clock_gettime(CLOCK_REALTIME, &tmo);
#ifdef DBUG
  printf("%d.%09d\n",tmo.tv_sec,tmo.tv_nsec);
  tmo.tv_sec+=2;
  int result = sem_timedwait(&_payload_sem, &tmo);

  clock_gettime(CLOCK_REALTIME, &tmo);
  printf("%d.%09d\n",tmo.tv_sec,tmo.tv_nsec);
#else
  tmo.tv_sec+=2;
  int result = sem_timedwait(&_payload_sem, &tmo);
#endif
  return result;
}

const Ami::Entry* Client::payload() const 
{
  return _cds.entry(_output_signature);
}

void Client::reset()
{
  if (_manager)
    _manager->configure();
}
