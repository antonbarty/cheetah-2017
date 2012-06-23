//
//  VClientManager manages the connection and disconnection of a client
//  to a server in the peer group.  Since there can be many servers 
//  processing data in parallel, all requests will be answered with 
//  multiple replies.  The last reply should be followed by a timeout.
//  Connection requests are sent to the ServerManagers of the peer group.
//  All other requests go to the allocated Servers.
//
#include "VClientManager.hh"

#include "ami/service/Sockaddr.hh"
#include "ami/service/Ins.hh"
#include "ami/service/Port.hh"

#include "ami/data/Cds.hh"
#include "ami/data/Discovery.hh"
#include "ami/data/Message.hh"

#include "ami/client/AbsClient.hh"
#include "ami/client/VClientSocket.hh"

#include "pdsdata/xtc/DetInfo.hh"

#include <errno.h>
#include <string.h>
#include <stdio.h>

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


VClientManager::VClientManager(unsigned   interface,
			       unsigned   serverGroup,
			       AbsClient& client) :
  VPoll      (1000),
  _interface (interface),
  _serverGroup(serverGroup),
  _socket    (new VClientSocket),
  _client    (client),
  _state     (Disconnected),
  _request   (0,Message::NoOp),
  _iovs      (new iovec[Step]),
  _buffer    (new char[BufferSize]),
  _discovery (new char[BufferSize])
{
  _socket->setrcvbuf(0x400000);  // payloads can be quite large
  manage(*this);
}


VClientManager::~VClientManager()
{
  disconnect();
  unmanage(*this);
  delete[] _iovs;
  delete[] _buffer;
  delete[] _discovery;
}

void VClientManager::request_payload()
{
  if (_state == Connected) {
    _request = Message(_request.id()+1,Message::PayloadReq);
    _socket->write(&_request, sizeof(_request));
  }
}

//
//  Connecting involves negotiating a destination port with the server group
//  and allocating a server on the peer connected to that port.  
//
void VClientManager::connect()
{
  _nconnected = 0;
  timeout(1000);

  Ins ins(_serverGroup, Port::serverPortBase());
  _socket->set_dst(ins, _interface); // Route::interface());

  _request = Message(_request.id()+1,Message::Connect,Port::serverPortBase()+1);
  _socket->write(&_request, sizeof(_request));
  _state = Connected;
}

//
//  Disconnecting closes the server on the peer
//
void VClientManager::disconnect()
{
  if (_state != Disconnected && _nconnected) {
    _request = Message(_request.id()+1,Message::Disconnect);
    _socket->write(&_request, sizeof(_request));
    _state = Disconnected;
  }
}

void VClientManager::discover()
{
  _request = Message(_request.id()+1,Message::DiscoverReq);
  _socket->write(&_request, sizeof(_request));
}

//
//  Build the configuration request from the client.  A description request
//  is implied.
//
void VClientManager::configure()
{
  int n = _client.configure(_iovs+1);
  _request = Message(_request.id()+1,Message::ConfigReq);
  _request.payload(_iovs+1,n);
  _iovs[0].iov_base = &_request;
  _iovs[0].iov_len  = sizeof(_request);
  _socket->writev(_iovs, n+1);
}

// Fd interface
int VClientManager::fd() const { return _socket->socket(); }

//
//  Replies belonging to the most recent request are harvested here.
//  The timeout is set to a finite value, so the last reply to the request
//  should be followed by a timeout response.
//
int VClientManager::processIo()
{
  timeout(Timeout);

  Message reply(0,Message::NoOp);
  _socket->peek(&reply);
  
  if (reply.type() == Message::Discover) { // unsolicited message
    int size = _socket->read(_discovery,BufferSize);
    //    dump(_discovery,size);
    DiscoveryRx rx(_discovery, size);
    _client.discovered(rx);
  }
  else if (reply.id() == _request.id()) {   // solicited message
    switch (reply.type()) {
    case Message::Reconnect:
      connect();
      break;
    case Message::Connect:
      if (reply.payload()==0) {
	unsigned port = _request.payload();
	printf("Connect attempt on port %d failed\n",port);
	// attempt connect on the next port
	_request = Message(_request.id()+1,Message::Connect,port+1);
	_socket->write(&_request, sizeof(_request));
      }
      else {
	_nconnected++;
      }
      break;
    case Message::Description: 
      _client.read_description(*_socket,reply.payload());
      _request = Message(_request.id()+1,_request.type());  // only need one reply
      break;
    case Message::Payload:     
      _client.read_payload(*_socket,reply.payload());
      break;
    case Message::PayloadFragment:
      if (_assembler.assemble(reply,*_socket))
 	_client.read_payload(_assembler,0);
      break;
    default:          
      break;
    }
  }
  else {
    printf("received id %d/%d type %d/%d\n",reply.id(),_request.id(),reply.type(),_request.type());
  }

  _socket->flush(); 
  return 1;
}

//
//  A timeout indicates the last reply to the most recent request.
//  The timeout is disabled until a reply to next request is received.
//
int VClientManager::processTmo()
{
  timeout(-1);

  switch(_request.type()) {
  case Message::Connect:
    if (_nconnected) {
      unsigned port = _request.payload();
      printf("Connect attempt on port %d succeeded\n",port);
      _socket->set_dst(Ins(_serverGroup,port),_interface);
      _client.connected();
    }
    else {
      printf("Connect failed.  Retry...\n");
      connect();
    }
    break;
  case Message::ConfigReq:
    _client.configured();
    break;
  case Message::PayloadReq:
    _client.process();
    break;
  case Message::DescriptionReq: 
  default:
    break;
  }
  return 1;
}
