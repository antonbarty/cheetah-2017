#ifndef Ami_FD_HH
#define Ami_FD_HH

namespace Ami {
  class Fd {
  public:
    virtual ~Fd() {}
    virtual int fd() const = 0;
    virtual int processIo() = 0;
    virtual int processIo(const char*,int) { return 1; }
  };
};

#endif
