//
//  Server handles all client requests for data description and payload
//
#include "Server.hh"

#include "ami/service/Socket.hh"
#include "ami/server/Factory.hh"
#include "ami/data/Discovery.hh"

#include <errno.h>

using namespace Ami;

static const int BufferSize = 32*1024;

Server::Server(Socket*         socket,
	       Factory&        factory,
	       const Message&  request) :
  _socket(socket),
  _iov   (new iovec[10]),
  _iovcnt(10),
  _reply (0,Message::NoOp),
  _cds   ("Server"),
  _factory(factory),
  _buffer (new char[BufferSize]),
  _described(false)
{
  _socket->setsndbuf(0x400000);  // payloads can be quite large

  _iov[0].iov_base = &_reply;
  _iov[0].iov_len  = sizeof(_reply);

//   _reply = request;
//   _socket->writev(_iov,1);

//   Message disc(0, Message::DiscoverReq);
//   processIo(reinterpret_cast<const char*>(&disc), sizeof(disc));
}

Server::~Server()
{
  _factory.configure(fd(),Message(0,Message::ConfigReq),0,_cds);
  delete _socket;
  delete[] _buffer;
  delete[] _iov;
}

int Server::fd() const { return _socket->socket(); }

int Server::processIo()
{
  int r = 1;
  Message request(0,Message::NoOp);
  int result = _socket->read(&request,sizeof(request));
  if (result != sizeof(request)) {
    printf("S processIo read result %d (skt %d): %s\n",
	   result,_socket->socket(),strerror(errno));
    return 0;
  }

//   printf("S request type %d id %d (skt %d)\n",
// 	 request.type(), request.id(), _socket->socket());

  switch(request.type()) {
  case Message::Disconnect:
    _described=false;
    r = 0;
    break;
  case Message::DiscoverReq:
    { DiscoveryTx tx(_factory.features(),_factory.discovery());
      int n = tx.niovs()+1;
      _adjust(n);
      tx.serialize(_iov+1);
      reply(request.id(), Message::Discover, n); }
    break;
  case Message::ConfigReq:
    _socket->read(_buffer,request.payload());
    _factory.configure(fd(), request,_buffer,_cds);
    _described = true;
  case Message::DescriptionReq:
    { int n = _cds.description()+1;
      _adjust(n);
      _cds.description(_iov+1);
      reply(request.id(), Message::Description, n); }
    break;
  case Message::PayloadReq:
    if (_described) {
      int n = _cds.payload()+1;
      _adjust(n);
      _cds.payload(_iov+1);
      reply(request.id(), Message::Payload, n); 
      //      _cds.invalidate_payload();
    }
    break;
  default:
    break;
  }

  return r;
}

int Server::processIo(const char* msg,int size)
{
  const Message& request = *reinterpret_cast<const Message*>(msg);
  switch(request.type()) {
  case Message::DiscoverReq:
    _described = false;
    { DiscoveryTx tx(_factory.features(),_factory.discovery());
      int n = tx.niovs()+1;
      _adjust(n);
      tx.serialize(_iov+1);
      reply(request.id(), Message::Discover, n); }
    break;
  default:
    break;
  }
  return 1;
}

void Server::_adjust    (int size)
{
  if (size > _iovcnt) {
    delete[] _iov;
    _iov = new iovec[size+1];
    _iov[0].iov_base = &_reply;
    _iov[0].iov_len  = sizeof(_reply);
    _iovcnt = size;
  }
}

void Server::reply(unsigned id, Message::Type type, unsigned cnt)
{
  _reply.id(id);
  _reply.type(type);
  _reply.payload(_iov+1,cnt-1);

//   printf("S reply type %d  id %d  payload %d\n",
// 	 _reply.type(),_reply.id(),_reply.payload());

  if (_socket->writev(_iov,cnt)<0)
    printf("Error in Server::reply writev writing %d bytes : %s\n",_reply.payload(),strerror(errno));
}
