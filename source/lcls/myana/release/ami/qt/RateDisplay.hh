#ifndef AmiQt_RateDisplay_hh
#define AmiQt_RateDisplay_hh

#include "ami/service/Timer.hh"

#include "ami/data/Cds.hh"

class iovec;
class QVBoxLayout;

namespace Ami {
  class DiscoveryRx;
  class Socket;
  class ClientManager;
  namespace Qt {
    class RateCalculator;
    class RateDisplay : public Timer {
    public:
      RateDisplay(ClientManager*);
      ~RateDisplay();
    public:
      void addLayout       (QVBoxLayout*);
      int  configure       (char*&);
      void discovered      (const DiscoveryRx&);
      void read_description(Socket&,int);
      void read_payload    (Socket&,int);
      void process         ();
    public:  // Timer interface
      void     expired();
      Task*    task   () { return _task; }
      unsigned duration  () const { return 1000; }
      unsigned repetitive() const { return 1; }
    private:
      ClientManager*  _manager;
      unsigned        _input;
      char*           _description;
      Cds             _cds;
      unsigned        _niovload;
      iovec*          _iovload;
      RateCalculator* _inputCalc;
      RateCalculator* _acceptCalc;
      Task*           _task;
      volatile bool   _ready;
    };
  };
};

#endif
