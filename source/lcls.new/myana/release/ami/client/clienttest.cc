#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "ami/client/AbsClient.hh"
#include "ami/client/VClientManager.hh"
#include "ami/data/ChannelID.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/Desc.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/Discovery.hh"
#include "ami/data/RawFilter.hh"
#include "ami/data/Average.hh"
#include "ami/service/Ins.hh"
#include "ami/service/Socket.hh"
#include "pdsdata/xtc/ClockTime.hh"

using namespace Ami;

typedef Pds::DetInfo DI;

static const int BufferSize = 0x8000;

class TestClient : public AbsClient {
public:
  TestClient() : _sem(Semaphore::EMPTY), _signature(0), _request(new char[0x8000]) {}
  ~TestClient() { delete[] _request; }
public:
  void wait() { _sem.take(); }
public:
  void connected()
  {
    printf("connected\n");
    _sem.give(); 
  }
  void discovered(const DiscoveryRx& rx)
  {
    printf("discovered\n");
    char acqname [128]; strcpy(acqname ,ChannelID::name(DI(0,DI::NoDetector,0,DI::Acqiris,0),0));
    char opalname[128]; strcpy(opalname,ChannelID::name(DI(0,DI::NoDetector,0,DI::Opal1000,0),0));

    for(const DescEntry* e = rx.entries(); e < rx.end(); 
	e = reinterpret_cast<const DescEntry*>
	  (reinterpret_cast<const char*>(e + e->size()))) {
      if (strcmp(e->name(),acqname)==0)
	_acq_id = e->signature();
      if (strcmp(e->name(),opalname)==0)
	_opal_id = e->signature();
    }
    printf("acq_id %d : opal_id %d\n",_acq_id,_opal_id);
  }
  int  configure       (iovec* iov) 
  {
    printf("configure\n");

    char* p = _request;
    { RawFilter filter;
      Average   op;
      ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						      ConfigureRequest::Discovery,
						      _acq_id,_signature++,filter,op);
      p += r.size(); }
    { RawFilter filter;
      Average   op;
      ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						      ConfigureRequest::Discovery,
						      _opal_id,_signature++,filter,op);
      p += r.size(); }
    iov[0].iov_base = _request;
    iov[0].iov_len  = p - _request;
    return 1;
  }
  int  configured      () 
  {
    printf("configured\n"); 
    _sem.give(); 
    return 0; 
  }
  void read_description(Socket&,int)
  {
    printf("description\n"); 
    _sem.give(); 
  }
  void read_payload    (Socket&,int)
  {
    printf("payload\n"); 
  }
  void process         () { printf("process\n"); }
private:
  Semaphore _sem;
  unsigned  _signature;
  char*     _request;
  const char* _discovery;
  unsigned    _discovery_size;
  unsigned    _acq_id, _opal_id;
};

int main(int argc, char **argv) 
{
  unsigned interface = 0;
  in_addr inp;
  if (inet_aton(argv[1], &inp))
    interface = ntohl(inp.s_addr);
  
  TestClient client;
  VClientManager manager(interface, 0xefff2000, client);
  manager.connect();
  client.wait();

  manager.configure();
  client.wait();

  timespec tv;
  tv.tv_sec  = 0;
  tv.tv_nsec = 100000000;
  while(1) {
    const int maxlen=128;
    char line[maxlen];
    char* result = fgets(line, maxlen, stdin);
    if (!result) {
      fprintf(stdout, "\nExiting\n");
      break;
    }
  }

  manager.disconnect();

  return 0;
}
