#ifndef Ami_Aggregator_hh
#define Ami_Aggregator_hh

#include "ami/client/AbsClient.hh"
#include "ami/data/Cds.hh"

class iovec;

namespace Ami {

  class BSocket;

  class Aggregator : public AbsClient {
  public:
    Aggregator(AbsClient&);
    ~Aggregator();
  public:
    void connected       () ;
    void disconnected    () ;
    int  configure       (iovec*) ;
    int  configured      () ;
    void discovered      (const DiscoveryRx&) ;
    void read_description(Socket&,int) ;
    void read_payload    (Socket&,int) ;
    void process         () ;
  private:
    AbsClient& _client;
    unsigned   _n;
    unsigned   _remaining;
    Cds        _cds;
    unsigned   _niovload;
    iovec*     _iovload;
    iovec*     _iovdesc;
    BSocket*   _buffer;
    enum { Init, Configured, Describing, Described, Processing } _state;
    double     _latest;
  };
};

#endif
