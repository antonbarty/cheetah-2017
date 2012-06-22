//
//  ClientManager manages the connection and disconnection of a client
//  to a server in the peer group.  Since there can be many servers 
//  processing data in parallel, all requests will be answered with 
//  multiple replies.  The last reply should be followed by a timeout.
//  Connection requests are sent to the ServerManagers of the peer group.
//  All other requests go to the allocated Servers.
//
#include "ClientManager.hh"

#include "ami/service/Sockaddr.hh"
#include "ami/service/Ins.hh"
#include "ami/service/Port.hh"
#include "ami/service/Poll.hh"
#include "ami/service/Task.hh"

#include "ami/data/Cds.hh"
#include "ami/data/Discovery.hh"
#include "ami/data/Message.hh"
#include "ami/data/Aggregator.hh"

#include "ami/client/AbsClient.hh"
#include "ami/client/ClientSocket.hh"
#include "ami/client/VClientSocket.hh"

#include "ami/server/VServerSocket.hh"

#include "pdsdata/xtc/DetInfo.hh"

#include <errno.h>
#include <string.h>
#include <stdio.h>

namespace Ami {
  //
  //  A class to listen for servers that come online
  //
  class ServerConnect : public Routine {
  public:
    ServerConnect(ClientManager& mgr,
		  Socket*        skt) :
      _task(new Task(TaskObject("cmco"))),
      _mgr(mgr), _skt(skt), _found(false) 
    {
      _task->call(this); 
    }
    ~ServerConnect() {
      _task->destroy_b();
      delete _skt;
    }
  public:
    void routine()
    {
      pollfd fds[1];
      fds[0].fd = _skt->socket();
      fds[0].events = POLLIN | POLLERR;

      if (poll(fds, 1, 1000)>0) {
        Message msg(0,Message::NoOp);
        if (_skt->read(&msg,sizeof(msg))==sizeof(msg))
          if (msg.type()==Message::Hello) {
            printf("Hello from socket %d\n",
		   _skt->socket());
            _found = true;
          }
      }
      else if (_found) {
        _found = false;
        _mgr.connect();
      }
      _task->call(this);
    }
  private:
    Task*          _task;
    ClientManager& _mgr;
    Socket*        _skt;
    bool           _found;
  };

  class ProxyConnect : public Routine {
  public:
    ProxyConnect(ClientManager& mgr,
                 Socket*&       skt,
                 const Ins&     ins) :
      _task(new Task(TaskObject("cmco"))),
      _mgr(mgr), _skt(skt), _ins(ins), _found(false) 
    {
      _task->call(this); 
    }
    ~ProxyConnect() {
      _task->destroy_b();
      delete _skt;
    }
  public:
    void routine()
    {
      if (_skt==0) {
        sleep(1);
        TSocket* skt = new TSocket;
        try {
          skt->connect(_ins);
          _skt = skt;
        }
        catch(Event& e) {
          delete skt;
        }
      }
      else {
        pollfd fds[1];
        fds[0].fd = _skt->socket();
        fds[0].events = POLLIN | POLLERR;

        if (poll(fds, 1, 1000)>0) {
          Message msg(0,Message::NoOp);
          int len = _skt->read(&msg,sizeof(msg));
          if (len==sizeof(msg)) {
            if (msg.type()==Message::Hello) {
              printf("Hello from socket %d\n",
                     _skt->socket());
              _found = true;
            }
          }
          else if (len<=0) {
            printf("Proxy offline\n");
            delete _skt;
            _skt = 0;
          }
        }
        else if (_found) {
          _found = false;
          _mgr.connect();
        }
      }
      _task->call(this);
    }
  private:
    Task*          _task;
    ClientManager& _mgr;
    Socket*&       _skt;
    Ins            _ins;
    bool           _found;
  };
};

using namespace Ami;

static const int Step = 20;
static const int Timeout = 20;  // milliseconds
static const int BufferSize = 0x8000;

static void dump(const char* payload, unsigned size) __attribute__((unused));

static void dump(const char* payload, unsigned size)
{
  const unsigned* p = (const unsigned*)payload;
  const unsigned* const e = p+(size>>2);
  for(unsigned k=0; p < e; k++,p++)
      printf("%08x%c", *p, (k%8)==7 ? '\n' : ' ');
  printf("\n");
}


ClientManager::ClientManager(unsigned   ppinterface,
			     unsigned   interface,
			     unsigned   serverGroup,
			     unsigned   short port,
			     AbsClient& client) :
  _client    (*new Aggregator(client)),
  _ppinterface(ppinterface),
  _port       (port),
  _poll      (new Poll(1000)),
  _state     (Disconnected),
  _request   (0,Message::NoOp),
  _iovs      (new iovec[Step]),
  _buffer_size(BufferSize),
  _buffer    (new char[BufferSize]),
  _discovery_size(BufferSize),
  _discovery (new char[BufferSize]),
  _task      (new Task(TaskObject("lstn"))),
  _listen_sem(Semaphore::EMPTY),
  _client_sem(Semaphore::EMPTY),
  _server    (serverGroup, Port::serverPort())
{
  bool mcast = Ins::is_multicast(serverGroup);
  printf("CM pp %x int %x grp %x mcast %c\n", 
	 ppinterface, interface, serverGroup,
	 mcast ? 'T':'F');

  if (mcast) {
    VClientSocket* so = new VClientSocket;
    so->set_dst(_server, interface);
    _connect = so;
  }
  else {
    // Connect to a proxy
    TSocket* so = new TSocket;
    so->connect(_server);
    _connect = so;
  }

  _listen = new TSocket;
  _port   = 0;
  while(_port!=port) {
    _port = port;
    try             { _listen->bind(Ins(ppinterface,_port)); }
    catch(Event& e) { 
      //        printf("bind error : %s : trying port %d\n",e.what(),++port); 
      ++port;
    }
  }
  _task->call(this);
  _listen_sem.take();
  
  if (mcast)
    _reconn = new ServerConnect(*this, new VServerSocket(_server, interface));
  else {
    _reconn = new ProxyConnect(*this, _connect, _server);
  }

  _poll->start();
}


ClientManager::~ClientManager()
{
  disconnect();
  delete[] _iovs;
  delete[] _buffer;
  delete[] _discovery;
  _task->destroy();
  _poll->stop();
  delete _poll;
  if (_reconn ) delete _reconn ;
  if (_listen ) delete _listen ;
  delete &_client;
}

void ClientManager::request_payload()
{
  if (_state == Connected) {
    _request = Message(_request.id()+1,Message::PayloadReq);
    _poll->bcast_out(reinterpret_cast<const char*>(&_request),
		     sizeof(_request));
  }
}

//
//  Connecting involves negotiating a destination port with the server group
//  and allocating a server on the peer connected to that port.  
//
void ClientManager::connect(bool svc)
{
  //  Remove previous connections
  unsigned n = nconnected();
  while(n)
    delete &_poll->fds(n--);

  if (_connect) {
    _request = Message(svc ? 1:0,Message::Connect,
                       _ppinterface,
                       _listen->ins().portId());
    _connect->write(&_request,sizeof(_request));
  }
}

int ClientManager::nconnected() const { return _poll->nfds(); }

//
//  Disconnecting closes the server on the peer
//
void ClientManager::disconnect()
{
  if (_state != Disconnected && nconnected()) {
    _request = Message(_request.id()+1,Message::Disconnect);
    _poll->bcast_out(reinterpret_cast<const char*>(&_request),
		     sizeof(_request));
    _state = Disconnected;
  }
}

void ClientManager::discover()
{
  _request = Message(_request.id()+1,Message::DiscoverReq);
  _poll->bcast_out(reinterpret_cast<const char*>(&_request),
		   sizeof(_request));
}

//
//  Build the configuration request from the client.  A description request
//  is implied.
//
void ClientManager::configure()
{
  int n = _client.configure(_iovs+1);
  _request = Message(_request.id()+1,Message::ConfigReq);
  _request.payload(_iovs+1,n);
  _iovs[0].iov_base = &_request;
  _iovs[0].iov_len  = sizeof(_request);
  _poll->bcast_out(_iovs, n+1);
  _client.configured();
}

void ClientManager::routine()
{
  while(1) {
    if (::listen(_listen->socket(),5)<0)
      printf("ClientManager listen failed\n");
    else {
      _listen_sem.give();
      Ami::Sockaddr name;
      unsigned length = name.sizeofName();
      int s = ::accept(_listen->socket(),name.name(), &length);
      if (s<0)
	printf("ClientManager accept failed\n");
      else {
	ClientSocket* cs = new ClientSocket(*this,s);
	Ins local  = cs->ins();
	Ins remote = name.get();
	_state = Connected;
      }
    }
  }
}

void ClientManager::add_client(ClientSocket& socket) 
{
//   printf("(%p) ClientManager::add_client %d (skt %d)\n",
// 	 this, nconnected(),socket.socket());
  _client.connected();
  _poll->manage(socket);
}

void ClientManager::remove_client(ClientSocket& socket)
{
  _poll->unmanage(socket);
  _client.disconnected();
//   printf("(%p) ClientManager::remove_client %d (skt %d)\n",
// 	 this,nconnected(),socket.fd());
  _client_sem.give();
}

void ClientManager::_flush_sockets(const Message& reply,
				   ClientSocket& socket)
{
  for(int i=0; i<nconnected(); i++) {
    ClientSocket& s = static_cast<ClientSocket&>(_poll->fds(i));
    if (&s != &socket) {
      Message r(0,Message::NoOp);
      s.read(&r,sizeof(r));
      if (r.type() != reply.type()) {
	printf("_flush_sockets type %d from socket %d\n",
	       r.type(), s.fd());
      }
      _flush_socket(s,r.payload());
    }
  }
}

void ClientManager::_flush_socket(ClientSocket& socket,
                                  int           remaining)
{
  while(remaining) {
    int sz = socket.read(_buffer,remaining < BufferSize ? 
                         remaining : BufferSize);
    remaining -= sz;
  } 
}

int ClientManager::handle_client_io(ClientSocket& socket)
{
  Message reply(0,Message::NoOp);
  if (socket.read(&reply,sizeof(reply))!=sizeof(reply)) {
//     printf("Error reading from skt %d : %s\n",
// 	   socket.socket(), strerror(errno));
    return 0;
  }

//   printf("(%p) handle_client_io type %d from socket %d\n",
// 	 this, reply.type(), socket.fd());

  if (reply.type() == Message::Discover) { // unsolicited message
    if (_discovery_size < reply.payload()) {
      printf("ClientManager: Increasing discovery buffer size %d to %d \n",
             _discovery_size, reply.payload());
      delete[] _discovery;
      _discovery = new char[_discovery_size=reply.payload()];
    }
    int size = socket.read(_discovery,reply.payload());
    //    dump(_discovery,size);
    DiscoveryRx rx(_discovery, size);
    _client.discovered(rx);
  }
  else if (reply.id() == _request.id()) {   // solicited message
  //  else if (1) { // solicited message
    switch (reply.type()) {
    case Message::Description: 
      _client.read_description(socket, reply.payload());
      //      _request = Message(_request.id()+1,_request.type());  // only need one reply
      break;
    case Message::Payload:     
      _client.read_payload(socket,reply.payload());
      _client.process();
      break;
    default:          
      break;
    }
  }
  else {
    //
    //  Sink the unsolicited message
    //
//     printf("(%p) received id %d/%d type %d/%d\n",
//  	   this, reply.id(),_request.id(),reply.type(),_request.type());
    switch (reply.type()) {
    case Message::Description: 
    case Message::Payload:     
      _flush_socket(socket,reply.payload());
      break;
    default:          
      break;
    }
  }
  return 1;
}
