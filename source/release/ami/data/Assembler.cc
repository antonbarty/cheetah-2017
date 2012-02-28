#include "Assembler.hh"

#include "ami/client/VClientSocket.hh"
#include "ami/data/Message.hh"
#include "ami/service/Sockaddr.hh"

#include <errno.h>

namespace Ami {
  class Assembler::Fragment {
  public:
    Fragment() {}
    ~Fragment() {}
  public:
    void fill(const Message& r,VClientSocket& s) { 
      _src = s.peer();
      memcpy(_header,&r,sizeof(r));
      _len=s.read(_payload,Assembler::ChunkSize);
    }
  public:
    const Sockaddr& src() const { return _src; }
    const Message& header() const { return *reinterpret_cast<const Message*>(_header); }
    unsigned       length() const { return _len; }
    const char*    payload() const { return _payload; }
  private:
    Sockaddr _src;
    unsigned _len;
    char _header [sizeof(Message)];
    char _payload[Assembler::ChunkSize];
  };
};

using namespace Ami;


Assembler::Assembler() {}

Assembler::~Assembler() {}

int Assembler::readv(const iovec* iov, int cnt)
{
//   for(int i=0; i<cnt; i++)
//     printf("iov[%d] %p/%x\n",i, iov[i].iov_base, iov[i].iov_len);

  const iovec* end = iov+cnt;
  iovec tiov = *iov;
  unsigned bytes = _current.front()->header().payload();
  unsigned remaining = bytes;
  unsigned offset=0;
  while(remaining) {
    for(std::list<Fragment*>::const_iterator it=_current.begin(); it!=_current.end(); it++) {
      if ((*it)->header().offset()==offset) {  // this is the next fragment
	unsigned rem  = (*it)->length();
	const char* p = (*it)->payload();
	offset       += rem;
	remaining    -= rem;
	while( tiov.iov_len < rem && iov < end ) {
// 	  printf("cpy %p %p %x\n",tiov.iov_base, p, tiov.iov_len);
	  memcpy(tiov.iov_base, p, tiov.iov_len);
	  p   += tiov.iov_len;
	  rem -= tiov.iov_len;
	  tiov = *++iov;
	}
	if (iov < end) {
	  // 	printf("cpy %p %p %x\n",tiov.iov_base, p, rem);
	  memcpy(tiov.iov_base,p,rem);
	  tiov.iov_len -= rem;
	  tiov.iov_base = (char*)tiov.iov_base+rem;
	}
	else if (rem) {
	  printf("Assembler at end of iov list with %d bytes remaining\n",rem);
	}
	_unused.push_back(*it);
	_current.remove(*it);
	break;
      }
    }
  }
  return bytes;
}

bool Assembler::assemble(const Message& r, VClientSocket& s)
{
  Fragment* f;
  if (_unused.size()) {
    f = _unused.back();
    _unused.pop_back();
  }
  else 
    f = new Fragment;
  f->fill(r,s);

  unsigned len = f->length();

  std::list<Fragment*> newlist;
  for(std::list<Fragment*>::iterator it = _fragments.begin(); it != _fragments.end(); it++) {
    bool remove=false;
    if ((*it)->src()==f->src()) {
      if ((*it)->header().id() == f->header().id()) {  // sum the matching fragment payload sizes 
	len += (*it)->length();
	_current.push_back(*it);
      }
      else if ((*it)->header().id() < f->header().id()) {  // discard older fragments
	Fragment* r = *it;
	remove=true;
	_unused.push_back(r);
      }
    }
    if (!remove) newlist.push_back(*it);
  }
  _fragments.clear();
  _fragments = newlist;

//   printf("assemble frag src %x  id %x  off %x/%x  len %x/%x\n",
// 	 f->src().get().address(), 
// 	 f->header().id(), f->header().offset(), f->header().payload(),
// 	 f->length(),len);

  if (len==f->header().payload()) {
    for(std::list<Fragment*>::const_iterator it = _current.begin(); it != _current.end(); it++)
      _fragments.remove(*it);
    _current.push_back(f);
    return true;
  }
  else {
    _current.clear();
    _fragments.push_back(f);
    return false;
  }
}

void Assembler::fragment(Socket& socket, const Message& m, iovec* _iov, unsigned cnt)
{
  unsigned offset    = 0;
  unsigned id        = m.id();
  unsigned payload   = m.payload();
  unsigned remaining = payload;
  unsigned chunk     = ChunkSize;
  iovec* iov = _iov;
  do {
//     for(unsigned i=0; i<cnt; i++)
//       printf(".iov[%d] %p/%x\n",i,_iov[i].iov_base,_iov[i].iov_len);
    
    int r = chunk;
    iovec* iiov = iov;
    unsigned s=0;
    while( r>0 )
      r -= (s=(++iiov)->iov_len);
    iiov->iov_len += r;
    
    Message rply(id, Message::PayloadFragment, payload, offset);
    iov->iov_base = &rply; iov->iov_len = sizeof(rply);

//     for(int i=0; i<=iiov-iov; i++)
//       printf("iov[%d] %p/%x\n",i,iov[i].iov_base,iov[i].iov_len);

    int bytes = socket.writev(iov,iiov-iov+1);
    if (bytes < 0) {
      printf("Error in Assembler::fragment writev writing %d bytes : %s\n",
	     chunk+sizeof(rply),strerror(errno));
      break;
    }
    else if ((unsigned)bytes != chunk+sizeof(rply)) {
      printf("Assembler::fragment wrote %d/%d bytes\n",
	     bytes, chunk+sizeof(rply));
      break;
    }
      
    iov        = iiov-1;
    iiov->iov_len  = -r;
    iiov->iov_base = (char*)iiov->iov_base+s+r; 
    offset    += chunk;
    remaining -= chunk;
    if (remaining < chunk) chunk = remaining;

//     printf("off %x/%x  rem %x  chk %x\n",
// 	   offset, payload, remaining, chunk);

  } while(remaining);
  _iov[0].iov_base = (void*)&m; _iov[0].iov_len = sizeof(m);
}
