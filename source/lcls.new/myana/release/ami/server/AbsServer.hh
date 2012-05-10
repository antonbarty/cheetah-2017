#ifndef Ami_AbsServer
#define Ami_AbsServer

class iovec;

namespace Ami {

  class AbsServer {
  public:
    virtual ~AbsServer() {}
  public:
    virtual int discover   ()       = 0;
    virtual int discover   (iovec*) = 0;
    virtual int description()       = 0;
    virtual int description(iovec*) = 0;
    virtual int payload    ()       = 0;
    virtual int payload    (iovec*) = 0;
  };

};

#endif
