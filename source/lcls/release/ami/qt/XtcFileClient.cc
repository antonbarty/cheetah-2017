#include "XtcFileClient.hh"

#include "ami/qt/FileSelect.hh"
#include "ami/app/XtcClient.hh"

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/xtc/ProcInfo.hh"

#include "ami/service/Task.hh"

#include <QtCore/QStringList>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QListWidget>
#include <QtGui/QApplication>

#include <glob.h>
#include <libgen.h>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

namespace Ami {
  namespace Qt {
    class ConfigureTask : public Routine {
    public:
      ConfigureTask(XtcFileClient& c) : _c(c) {}
      ~ConfigureTask() {}
    public:
      void routine() { _c.configure(); delete this; }
    private:
      XtcFileClient& _c;
    };
  };
};

using namespace Ami::Qt;

static void insert_transition(Pds::Dgram* dg, Pds::TransitionId::Value tr)
{
  new((void*)&dg->seq) Pds::Sequence(Pds::Sequence::Event, tr, Pds::ClockTime(0,0), Pds::TimeStamp(0,0,0,0));
  new((char*)&dg->xtc) Pds::Xtc(Pds::TypeId(Pds::TypeId::Id_Xtc,0),Pds::ProcInfo(Pds::Level::Recorder,0,0));
}

static void dump(Pds::Dgram* dg)
{
  char buff[128];
  time_t t = dg->seq.clock().seconds();
  strftime(buff,128,"%H:%M:%S",localtime(&t));
  printf("%s %08x/%08x %s extent %x damage %x\n",
	 buff,
	 dg->seq.stamp().fiducials(),dg->seq.stamp().vector(),
	 Pds::TransitionId::name(dg->seq.service()),
	 dg->xtc.extent, dg->xtc.damage.value());
}

static const unsigned _nodes=4;

XtcFileClient::XtcFileClient(Ami::XtcClient& client,
			     const char* basedir) :
  QWidget (0,::Qt::Window),
  _client (client),
  _basedir(basedir),
  _task   (new Task(TaskObject("amiqt")))
{
  QStringList experiments;
  {
    glob_t g;
    std::string gpath(_basedir);
    gpath += "/*";
    glob(gpath.c_str(),GLOB_ONLYDIR,0,&g);
    
    for(unsigned i=0; i<g.gl_pathc; i++)
      experiments << basename(g.gl_pathv[i]);
    globfree(&g);
  }

  _expt_select = new QListWidget;
  _expt_select->addItems(experiments);

  _runB  = new QPushButton("Run");
  QPushButton* exitB = new QPushButton("Exit");

  QVBoxLayout* l = new QVBoxLayout;
  l->addWidget(_expt_select);
  l->addWidget(_file_select = new FileSelect(0,QStringList()));
  { QHBoxLayout* l1 = new QHBoxLayout;
    l1->addStretch();
    l1->addWidget(_runB );
    l1->addStretch();
    l1->addWidget( exitB );
    l1->addStretch();
    l ->addLayout(l1); }
  setLayout(l);

  connect(_expt_select, SIGNAL(currentTextChanged(const QString&)), this, SLOT(select_expt(const QString&)));
  connect(_file_select, SIGNAL(run_selected()), this, SLOT(configure_run()));
  connect(_runB, SIGNAL(clicked()), this, SLOT(run()));
  connect(this , SIGNAL(done())   , this, SLOT(ready()));
  //  Infinite playback mode
  //  connect(this , SIGNAL(done())   , this, SLOT(run()));
  connect(exitB, SIGNAL(clicked()), qApp, SLOT(closeAllWindows()));
}

XtcFileClient::~XtcFileClient()
{
  _task->destroy();
}

void XtcFileClient::select_expt(const QString& expt)
{
  QStringList paths;
  paths << QString("%1/%2").arg(_basedir).arg(expt);
  paths << QString("%1/%2/xtc").arg(_basedir).arg(expt);

  _file_select->change_path_list(paths);
}

void XtcFileClient::configure_run()
{
  _task->call(new ConfigureTask(*this));
}

void XtcFileClient::configure()
{
  const char* fname = qPrintable(_file_select->paths().at(0));

  int fd = ::open(fname,O_LARGEFILE,O_RDONLY);
  if (fd == -1) {
    printf("Error opening file %s : %s\n",fname,strerror(errno));
    return;
  }
  else {
    printf("Opened file %s\n",fname);
  }

  char* buffer = new char[0x800000];
  Pds::Dgram* dg = (Pds::Dgram*)buffer;

  insert_transition(dg, TransitionId::Map);
  _client.processDgram(dg);

  //  read configure transition
  ::read(fd,dg, sizeof(Dgram));
  if (::read(fd,dg->xtc.payload(), dg->xtc.sizeofPayload()) != dg->xtc.sizeofPayload()) 
    printf("Unexpected eof in %s\n",fname);
  else
    _client.processDgram(dg);

  insert_transition(dg, TransitionId::Unconfigure);
  _client.processDgram(dg);

  insert_transition(dg, TransitionId::Unmap);
  _client.processDgram(dg);

  ::close(fd);
  delete[] buffer;
}

void XtcFileClient::run() 
{
  _runB->setEnabled(false);
  _task->call(this); 
}

void XtcFileClient::ready() 
{
  _runB->setEnabled(true);
}

//
//  We don't yet worry about chunked files
//
void XtcFileClient::routine()
{
  // process a set of files
  QStringList files = _file_select->paths();

  const unsigned nfiles = files.size();

  int* fd = new int[nfiles];
  for(unsigned i=0; i<nfiles; i++) {
    fd[i] = ::open(qPrintable(files.at(i)),O_LARGEFILE,O_RDONLY);
    if (fd[i] == -1) {
      printf("Error opening file %s\n",qPrintable(files.at(i)));
      return;
    }
    else {
      printf("Opened file %s\n",qPrintable(files.at(i)));
    }
  }  

  char* buffer = new char[0x800000];
  Pds::Dgram* dg = (Pds::Dgram*)buffer;

  Pds::Dgram* hdr = new Pds::Dgram[nfiles];

  //  insert a Map transition
  insert_transition(dg, TransitionId::Map);
  _client.processDgram(dg);

  for(unsigned k=0; k<nfiles; k++)
    ::read(fd[k], &hdr[k], sizeof(Dgram));

  bool ldone=false;
  do {
    //
    //  Process L1A with lowest clock time first
    //
    unsigned i = 0;
    ClockTime tmin(-1,-1);
    for(unsigned k=0; k<nfiles; k++) {
      if (hdr[k].seq.service()==Pds::TransitionId::L1Accept &&
	  tmin > hdr[k].seq.clock())
	tmin = hdr[i=k].seq.clock();
    }

    memcpy(dg, &hdr[i], sizeof(Pds::Dgram));
    if (::read(fd[i], dg->xtc.payload(), dg->xtc.sizeofPayload()) != dg->xtc.sizeofPayload()) {
      printf("Unexpected eof in %s\n",qPrintable(files.at(i)));
      break;
    }

    _client.processDgram(dg);

    if (dg->seq.service() != Pds::TransitionId::L1Accept) {

      dump(dg);
      
      for(unsigned k=0; k<nfiles; k++) {
	if (k != i) {
	  off_t off = hdr[k].xtc.sizeofPayload();
	  lseek64(fd[k], off, SEEK_CUR);
	}
	if (::read(fd[k], &hdr[k], sizeof(Dgram)) != sizeof(Dgram)) {
	  ldone=true;
	  printf("eof in %s\n",qPrintable(files.at(k)));
	}
      }
    }
    else {
      if (::read(fd[i], &hdr[i], sizeof(Dgram)) != sizeof(Dgram)) {
	ldone=true;
	printf("Unexpected eof in %s\n",qPrintable(files.at(i)));
      }
    }
  } while(!ldone);

  insert_transition(dg, TransitionId::Unconfigure);
  _client.processDgram(dg);

  insert_transition(dg, TransitionId::Unmap);
  _client.processDgram(dg);

  for(unsigned i=0; i<nfiles; i++) {
    printf("Closing file %s at position %lld\n",
	   qPrintable(files.at(i)),
	   lseek64(fd[i],off_t(0),SEEK_CUR));
    ::close(fd[i]);
  }

  delete[] hdr;
  delete[] buffer;
  delete[] fd;

  emit done();
}
