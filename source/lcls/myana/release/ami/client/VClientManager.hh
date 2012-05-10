#ifndef Ami_VClientManager_hh
#define Ami_VClientManager_hh

#include "ami/data/Assembler.hh"
#include "ami/data/Message.hh"
#include "ami/service/VPoll.hh"
#include "ami/service/Fd.hh"

#include <list>
using std::list;

class iovec;

namespace Ami {

  class AbsClient;
  class ConsumerClient;
  class VClientSocket;

  class VClientManager : public VPoll,
			 public Fd {
  public:
    VClientManager(unsigned   interface,
		   unsigned   serverGroup,
		   AbsClient& client);
    ~VClientManager();
  public:
    void connect   ();  // Connect to a server group
    void disconnect();  // Disconnect from a server group
    void discover  ();
    void configure ();
    void request_description();
    void request_payload    ();
  private:    // VPoll interface
    virtual int processTmo();
  public:    // Fd interface
    virtual int fd() const;
    virtual int processIo();
  private:
    enum State { Disconnected, Connected };
    unsigned        _interface;
    unsigned        _serverGroup;
    VClientSocket*  _socket;
    AbsClient&      _client;
    State           _state;
    Message         _request;
    iovec*          _iovs;
    char*           _buffer;
    char*           _discovery;
    unsigned        _nconnected;
    Assembler       _assembler;
  };

};

#endif
    
