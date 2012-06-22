#ifndef Ami_Assembler_hh
#define Ami_Assembler_hh

#include "ami/service/Socket.hh"

#include <list>

namespace Ami {
  class Message;
  class VClientSocket;
  class Assembler : public Socket {
  public:
    enum { ChunkSize=0x7f00 };
  public:
    Assembler();
    ~Assembler();
  public:  // Socket interface
    int readv(const iovec*,int);
  public:  // Fragment reassembly
    bool assemble(const Message&,VClientSocket&);
  public:  // Fragmentation
    static void fragment(Socket&,const Message&,iovec*,unsigned);
  private:
    class Fragment;
    std::list<Fragment*> _fragments;
    std::list<Fragment*> _unused;
    std::list<Fragment*> _current;
  };
};

#endif
