#include "ami/service/VPoll.hh"

#include "ami/service/Fd.hh"
#include "ami/service/Loopback.hh"

#include "ami/service/Task.hh"
#include "ami/service/TaskObject.hh"

#include <errno.h>
#include <string.h>

using namespace Ami;


VPoll::VPoll(int timeout) :
  _timeout (timeout),
  _task    (new Task(TaskObject("vmonMgr"))),
  _loopback(new Loopback),
  _ofd     (0)
{
  _pfd[0].fd = _loopback->socket();
  _pfd[0].events = POLLIN;
  _pfd[0].revents = 0;
}


VPoll::~VPoll()
{
  _task->destroy();
}


void VPoll::manage(Fd& fd)
{
  _ofd = &fd;
  _pfd[1].fd = fd.fd();
  _pfd[1].events = POLLIN;
  _pfd[1].revents = 0;
  _task->call(this);
}

void VPoll::unmanage(Fd& fd)
{
  int msg=0;
  _loopback->write(&msg,sizeof(msg));
}

int VPoll::poll()
{
  int result = 1;
  if (::poll(_pfd, 2, _timeout) > 0) {
    if (_pfd[1].revents & (POLLIN | POLLERR))
      result = _ofd->processIo();
    else if (_pfd[0].revents & (POLLIN | POLLERR)) {
      int cmd;
      _loopback->read(&cmd,sizeof(cmd));
      _ofd = 0;
      result = 0;
    }
  }
  else {
    result = processTmo();
  }
  return result;
}

void VPoll::routine()
{
  while(poll());
}

int  VPoll::timeout() const { return _timeout; }

void VPoll::timeout(int tmo) { _timeout=tmo; }
