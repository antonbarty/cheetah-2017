#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "bldIpimbClient.hh"
#include "pdsapp/config/Experiment.hh"
#include "pdsapp/config/Reconfig_Ui.hh"
#include "pdsapp/blv/IdleControlMsg.hh"
#include "ami/service/TSocket.hh"

#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QFont>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>

#define IPIMB_CONFIG_DB     "/reg/g/pcds/dist/pds/sharedIpimb/configdb"
#define IPIMB_HOSTNAME      "daq-sxr-nh2sb1ipm01"
#define CONTROL_PORT        5727

using namespace Ami::Blv;

ConfigSelect::ConfigSelect(unsigned     interface,
			   const SList& servers,
			   const char*  db_path) :
  QGroupBox("Control"),
  _servers (servers),
  _expt    (Pds_ConfigDb::Path(db_path)),
  _sendEnb(0)
{
  _expt.read();

  _reconfig = new Pds_ConfigDb::Reconfig_Ui(this, _expt);

  QPushButton* bEdit = new QPushButton("Edit");

  QVBoxLayout* layout = new QVBoxLayout;
  { QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addStretch();
    layout1->addWidget(new QLabel("Mode"));
    layout1->addWidget(_runType = new QComboBox);
    layout1->addStretch();
    layout->addLayout(layout1); }
  { QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addStretch();
    layout1->addWidget(bEdit);
    layout1->addStretch();
    layout->addLayout(layout1); }
  setLayout(layout);

  connect(bEdit   , SIGNAL(clicked()),                 _reconfig, SLOT(show()));
  connect(_runType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(set_run_type(const QString&)));
  connect(_reconfig,SIGNAL(changed()),                 this, SLOT(update()));

  read_db();
  set_run_type(_runType->currentText());
}

ConfigSelect::~ConfigSelect() 
{
}

void ConfigSelect::set_run_type(const QString& run_type)
{
  _reconfig->set_run_type(run_type);

  const Pds_ConfigDb::TableEntry* e = _expt.table().get_top_entry(qPrintable(run_type));
  if (e) {
    _run_key = strtoul(e->key().c_str(),NULL,16);
    _reconfigure();
  }
}

void ConfigSelect::update()
{
  read_db();

  const Pds_ConfigDb::TableEntry* e = _expt.table().get_top_entry(qPrintable(_runType->currentText()));
  _run_key = strtoul(e->key().c_str(),NULL,16);
  _reconfigure();
}

void ConfigSelect::read_db()
{
  QString type(_runType->currentText());

  //  _expt.read();
  bool ok = 
    disconnect(_runType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(set_run_type(const QString&)));
  _runType->clear();
  const list<Pds_ConfigDb::TableEntry>& l = _expt.table().entries();
  for(list<Pds_ConfigDb::TableEntry>::const_iterator iter = l.begin(); iter != l.end(); ++iter) {
    QString str(iter->name().c_str());
    _runType->addItem(str);
  }
  if (ok) 
    connect(_runType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(set_run_type(const QString&)));

  int index=0;
  for(list<Pds_ConfigDb::TableEntry>::const_iterator iter = l.begin(); iter != l.end(); ++iter, ++index) {
    QString str(iter->name().c_str());
    if (str == type) {
      _runType->setCurrentIndex(index);
      return;
    }
  }
  _runType->setCurrentIndex(0);
}

void ConfigSelect::_reconfigure()
{
  Pds::IdleControlMsg msg;
  sprintf(msg.dbpath,"%s/keys",_expt.path().base().c_str());
  msg.key = _run_key;

  for(SList::const_iterator it = _servers.begin(); it != _servers.end(); it++) {
    TSocket s;
    const unsigned short lport = 5730;
    Ins local(lport);
    s.bind(local);

    struct hostent* hent = gethostbyname(it->host);
    if (!hent) 
      printf("Failed to lookup entry for host %s\n",it->host);
    else {
      int ip_host = ntohl(*(unsigned*)hent->h_addr);
      Ins ins(ip_host, it->port);
      try {
        s.connect(ins);
        //if(_sendEnb)	
          s.write(&msg, sizeof(msg));
        //else
        //  _sendEnb = 1;
      } 
      catch (Event& e) {
        printf("Failed to reconfigure %s (%s)\n",it->name,it->host);
      }
    }
  }
}


Control::Control(const QString& label,
		 unsigned interface,
		 const SList& servers,
		 const char* db) :
  QtPWidget   (0)
{
  setWindowTitle(label);
  setAttribute(::Qt::WA_DeleteOnClose, false);

  QVBoxLayout* l = new QVBoxLayout;
  { QLabel* title = new QLabel(label);
    title->setWordWrap(true);
    QFont font = title->font();
    font.setPointSize(font.pointSize()+8);
    title->setFont(font);
    l->addWidget(title,0,::Qt::AlignHCenter); }

  l->addWidget(new ConfigSelect  (interface, servers, db));
  setLayout(l);
}


unsigned parse_interface(char* iarg)
{
  unsigned interface = 0;
  if (iarg[0]<'0' || iarg[0]>'9') {
    int skt = socket(AF_INET, SOCK_DGRAM, 0);
    if (skt<0) {
      perror("Failed to open socket\n");
      exit(1);
    }
    ifreq ifr;
    strcpy( ifr.ifr_name, iarg);
    if (ioctl( skt, SIOCGIFADDR, (char*)&ifr)==0)
      interface = ntohl( *(unsigned*)&(ifr.ifr_addr.sa_data[2]) );
    else {
      printf("Cannot get IP address for network interface %s.\n",iarg);
      exit(1);
    }
    printf("Using interface %s (%d.%d.%d.%d)\n",
	   iarg,
	   (interface>>24)&0xff,
	   (interface>>16)&0xff,
	   (interface>> 8)&0xff,
	   (interface>> 0)&0xff);
    close(skt);
  }
  else {
    in_addr inp;
    if (inet_aton(iarg, &inp))
      interface = ntohl(inp.s_addr);
  }
  return interface;
}

static void showUsage()
{
    printf( "Usage:  bldIpimbClient [-f <ipimbPortMapFile>] [-c <Ipimb Config DB>] [-i <Interface Name/Ip>] [-o <opcode>] [-h] \n" 
      " Options:\n"
      "   -h                        : Show Usage\n"
      "   -i <Interface Name/IP>    : Set the network Interface/IP for communication. Default: 'eth0' \n"
      "   -c <Config DB Path>       : Path for IPIMB Config DB. Default@ [%s] \n"
      "   -p <Control Port>         : Control Port for Remote Configuration over Cds Interface.Default: %u \n"
      "   -H <IPIMB Hostnames>      : Hostnames to communicate to (comma-separated list. Default : %s\n",IPIMB_CONFIG_DB,CONTROL_PORT,IPIMB_HOSTNAME );
    
}

int main(int argc, char **argv) 
{
  unsigned interface   = parse_interface("eth0"); //0x7f000001;
  unsigned controlPort = CONTROL_PORT;
  char* configdb = IPIMB_CONFIG_DB;
  char* hostName = IPIMB_HOSTNAME;
  
  extern char* optarg;
  int c;
  while ( (c=getopt( argc, argv, "i:c:H:p:h?")) != EOF ) {
    switch(c) {
    case 'i':
      interface = parse_interface(optarg);
      break;
    case 'c':
      configdb = optarg;
      break;
    case 'H':
      hostName = optarg;
      break;
    case 'p':
      controlPort = strtoul(optarg, NULL, 0);
      break;	  
    case 'h':
    case '?':
      showUsage();
      return 1;
    }
  } 
  
  QApplication app(argc, argv);

  SList servers;
  Ami::Blv::ServerEntry entry;
  unsigned det;
  char* host = strtok(hostName,",");
  do {
    sprintf(entry.name,host);
    sprintf(entry.host,host);
    det = (unsigned) Pds::DetInfo::XppSb1Ipm;
    entry.port = controlPort;
    entry.info = Pds::DetInfo(0,(Pds::DetInfo::Detector)det,1,Pds::DetInfo::Ipimb,0);
    servers.push_back(entry);  
  } while((host=strtok(NULL,",")));

  Control* control = new Control("BLD IPIMB Configuration", interface,servers,configdb);
  control->show();

  app.exec();

  return 0;
}
