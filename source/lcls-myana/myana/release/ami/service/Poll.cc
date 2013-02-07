#include "ami/service/Poll.hh"

#include "ami/service/Fd.hh"
#include "ami/service/Loopback.hh"

#include "ami/service/Task.hh"
#include "ami/service/TaskObject.hh"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

using namespace Ami;

const int Step=32;
const int BufferSize=0x100000;

enum LoopbackMsg { BroadcastIn, BroadcastOut, Shutdown, PostIn };

Poll::Poll(int timeout) :
  _timeout (timeout),
  _task    (new Task(TaskObject("vmonMgr"))),
  _loopback(new Loopback),
  _nfds    (1),
  _maxfds  (Step),
  _ofd     (new    Fd*[Step]),
  _pfd     (new pollfd[Step]),
  _sem     (Semaphore::EMPTY),
  _buffer  (new char[BufferSize])
{
  _pfd[0].fd = _loopback->socket();
  _pfd[0].events = POLLIN | POLLERR;
  _pfd[0].revents = 0;
}


Poll::~Poll()
{
  _task->destroy_b();
  delete[] _ofd;
  delete[] _pfd;
  delete[] _buffer;
}

void Poll::start()
{
  _task->call(this);
}

void Poll::stop()
{
  int msg=Shutdown;
  _loopback->write(&msg,sizeof(msg));
  _sem.take();
}

//
//  Broadcast a message to all remote endpoints
//
void Poll::bcast_out(const char* msg, int size)
{
  bcast(msg, size, BroadcastOut);
}

void Poll::bcast_out(const iovec* iov, int len)
{
  bcast(iov, len, BroadcastOut);
}

void Poll::bcast_in (const char* msg, int size)
{
  bcast(msg, size, BroadcastIn);
}

void Poll::bcast    (const char* msg, int size, int hdr)
{
  int iovcnt=2;
  iovec* iov = new iovec[iovcnt];
  iov[0].iov_base = &hdr      ; iov[0].iov_len = sizeof(hdr);
  iov[1].iov_base = (void*)msg; iov[1].iov_len = size;
  _loopback->writev(iov,iovcnt);
  delete[] iov;
//   _loopback->write(&hdr ,sizeof(hdr));
//   _loopback->write(&bsiz,sizeof(size));
//   _loopback->write(msg  ,size);
}

void Poll::bcast    (const iovec* iov, int len, int hdr)
{
  int iovcnt = len+1;
  iovec* niov = new iovec[iovcnt];
  niov[0].iov_base = &hdr ; niov[0].iov_len = sizeof(hdr);
  for(int i=0; i<len; i++) {
    niov[i+1].iov_base = iov[i].iov_base;
    niov[i+1].iov_len  = iov[i].iov_len;
  }
  _loopback->writev(niov ,iovcnt);
  delete[] niov;
//   _loopback->write(&hdr ,sizeof(hdr));
//   _loopback->write(&size,sizeof(size));
//   _loopback->writev(iov ,len);
}

//
//  Post a message to ourselves in the polling thread
//
void Poll::post    (const char* msg, int size)
{
  printf("Poll::post\n");
  int hdr = PostIn;
  int iovcnt=2;
  iovec* iov = new iovec[iovcnt];
  iov[0].iov_base = &hdr      ; iov[0].iov_len = sizeof(hdr);
  iov[1].iov_base = (void*)msg; iov[1].iov_len = size;
  _loopback->writev(iov,iovcnt);
  delete[] iov;
}

//
//  Should only be called before "start" or within poll thread;
//  i.e. from processIo()
//
void Poll::manage(Fd& fd)
{
  unsigned available = 0;
  for (unsigned short n=1; n<_nfds; n++) {
    if (!_ofd[n] && !available) available = n;
    if (_ofd[n] == &fd) return;
  }
  if (!available) {
    if (_nfds == _maxfds) adjust();
    available = _nfds++;
  }
  _ofd[available] = &fd;
  _pfd[available].fd = fd.fd();
  _pfd[available].events = POLLIN | POLLERR;
  _pfd[available].revents = 0;
}

void Poll::unmanage(Fd& fd)
{
  for (unsigned short n=1; n<_nfds; n++) {
    if (_ofd[n] == &fd) {
      _ofd[n] = 0;
      _pfd[n].fd = -1;
      _pfd[n].events = 0;
      _pfd[n].revents = 0;
    }
  }
}

int Poll::poll()
{
  int result = 1;
  if (::poll(_pfd, _nfds, _timeout) > 0) {
    if (_pfd[0].revents & (POLLIN | POLLERR)) {
      int size;
      int& cmd = *reinterpret_cast<int*>(_buffer);
      if ((size=_loopback->read(_buffer,BufferSize))<0)
	printf("Error reading loopback\n");
      else if (cmd==Shutdown)
	result = 0;
      else {
	size -= sizeof(int);
	if (size<0)
	  printf("Error reading bcast\n");
	else {
	  const char* payload = _buffer+sizeof(int);
          if (cmd==BroadcastIn || cmd==BroadcastOut) {
            for (unsigned short n=1; n<_nfds; n++) {
              if (_ofd[n])
                if (cmd==BroadcastOut)
                  ::write(_ofd[n]->fd(), payload, size);
                else if (!_ofd[n]->processIo(payload,size)) {
                  Fd* fd = _ofd[n];
                  unmanage(*fd);
                  delete fd;
                }
            }
          }
          else if (cmd==PostIn)
            processIn(payload,size);
	}
      }
    }
    for (unsigned short n=1; n<_nfds; n++) {
      if (_ofd[n] && (_pfd[n].revents & (POLLIN | POLLERR)))
	if (!_ofd[n]->processIo()) {
          Fd* fd = _ofd[n];
          unmanage(*fd);
          delete fd;
        }
    }
  }
  else {
    result = processTmo();
  }
  return result;
}

int Poll::processTmo() 
{
  return 1; 
}

int Poll::processIn(const char*, int)
{
  return 1;
}

void Poll::routine()
{
  while(poll());
  _sem.give();
}

int  Poll::nfds() const 
{
  int nfds(0);
  for(unsigned short i=1; i<_nfds; i++) {
    if (_ofd[i])
      nfds++;
  }
  return nfds;
}

Fd&  Poll::fds (int i) const { return *_ofd[i]; }

int  Poll::timeout() const { return _timeout; }

void Poll::timeout(int tmo) { _timeout=tmo; }

void Poll::adjust()
{
  unsigned short maxfds = _maxfds + Step;

  Fd** ofd = new Fd*[maxfds];
  memcpy(ofd, _ofd, _nfds*sizeof(Fd*));
  delete [] _ofd;
  _ofd = ofd;

  pollfd* pfd = new pollfd[maxfds];
  memcpy(pfd, _pfd, _nfds*sizeof(pollfd));
  delete [] _pfd;
  _pfd = pfd;

  _maxfds = maxfds;
}
