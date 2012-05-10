#ifndef Ami_ClientManager_hh
#define Ami_ClientManager_hh

#include "ami/data/Message.hh"
#include "ami/service/Ins.hh"
#include "ami/service/Routine.hh"
#include "ami/service/Semaphore.hh"

#include <list>
using std::list;

class iovec;

namespace Ami {

  class AbsClient;
  class ClientSocket;
  class VClientSocket;
  class Poll;
  class Task;
  class Socket;
  class TSocket;
  class ConnectRoutine;

  class ClientManager : public Routine {
  public:
    ClientManager(unsigned   ppinterface,
		  unsigned   interface,
		  unsigned   serverGroup,
		  unsigned short port,
		  AbsClient& client);
    ~ClientManager();
  public:
    void connect   (bool svc=false);  // Connect to a server group
    void disconnect();  // Disconnect from a server group
    void discover  ();
    void configure ();
    void request_description();
    void request_payload    ();
  public:    // Routine interface
    virtual void routine();
  public:
    void add_client      (ClientSocket&);
    void remove_client   (ClientSocket&);
    int  handle_client_io(ClientSocket&);
    int  nconnected      () const;
  private:
    void _flush_sockets(const Message&, ClientSocket&);
    void _flush_socket (ClientSocket&, int);
  private:
    enum State { Disconnected, Connected };
    AbsClient&      _client;
    int             _ppinterface;
    unsigned short  _port;
    Poll*           _poll;
    State           _state;
    Message         _request;
    iovec*          _iovs;
    unsigned        _buffer_size;
    char*           _buffer;
    unsigned        _discovery_size;
    char*           _discovery;
    
    Task*           _task;
    TSocket*        _listen;
    Socket*         _connect;
    Semaphore       _listen_sem;
    Semaphore       _client_sem;
    Ins             _server;
    ConnectRoutine* _reconn;
  };

};

#endif
    
