#include <stdlib.h>
#include <stdio.h>

#include "ami/app/XtcShmClient.hh"
#include "ami/app/XtcClient.hh"
#include "ami/service/Routine.hh"
#include "ami/service/Task.hh"

namespace Ami {
  class ShmTask : public Routine {
  public:
    ShmTask(XtcShmClient& c, char* tag, int id) : 
      _c(c), _tag(tag), _id(id) {}
    ~ShmTask() {}
  public:
    void routine() { _c.run(_tag,_id,_id); delete this; }
  private:
    XtcShmClient& _c;
    char*         _tag;
    int           _id;
  };
};

using namespace Ami;

XtcShmClient::XtcShmClient(XtcClient& client, char * tag, int id) :
  _client(client),
  _sem(Semaphore::EMPTY),
  _task(new Task(TaskObject("amishm")))
{
  if (::pipe(_pipefd)<0)
    perror("XtcShmClient pipe failed");

  _task->call(new ShmTask(*this,tag,id));
}

int XtcShmClient::processDgram(Dgram* dg)
{
  ::write(_pipefd[1],&dg,sizeof(dg));
  _sem.take();
  return 0;
}

int XtcShmClient::processIo()
{
  Dgram* dg;
  ::read(_pipefd[0],&dg,sizeof(dg));
  _client.processDgram(dg);
  _sem.give();
  return 1;
}

int XtcShmClient::fd() const { return _pipefd[0]; }

