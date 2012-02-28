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

#include "blvclient.hh"
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

using namespace Ami::Blv;

DetectorListItem::DetectorListItem(QListWidget*        parent,
				   const QString&      dlabel,
				   const Pds::DetInfo& dinfo, 
				   unsigned            interface,
				   const char*         host,
				   unsigned            port ) :
  QListWidgetItem(dlabel, parent),
  info           (dinfo),
  _parent        (parent),
  _interface     (interface),
  _port          (port),
  _client        (0),
  _manager       (0)
{
  setTextAlignment(::Qt::AlignHCenter);

  struct hostent* hent = gethostbyname(host);
  if (!hent) 
    printf("Failed to lookup entry for host %s\n",host);
  else
    _ip_host = ntohl(*(unsigned*)hent->h_addr);
}

void DetectorListItem::show() { 
  if (_client)
    _client->show();
  else {

    _client = new Ami::Qt::ImageClient(_parent, info, 0);
    
    _manager = new ClientManager(_interface, _interface, _ip_host, _port,
				 *_client);
    _client->managed(*_manager);
  }
}

void DetectorListItem::hide() { if (_client) _client->hide(); }

DetectorSelect::DetectorSelect(unsigned interface,
			       const SList& servers) :
  QGroupBox   ("Data"),
  _clientPort (Port::clientPortBase()),
  _last_item  (0)
{
  QVBoxLayout* layout = new QVBoxLayout;    

  layout->addWidget(_detList = new QListWidget(this));
  for(SList::const_iterator it = servers.begin(); it != servers.end(); it++)
    if (strcmp(it->name,"EVR"))
      new DetectorListItem(_detList, it->name, it->info, 
			   interface, it->host, _clientPort++);

  connect(_detList, SIGNAL(itemClicked(QListWidgetItem*)), 
	  this, SLOT(show_detector(QListWidgetItem*)));

  setLayout(layout);

  _reconnect_timer = new QTimer(this);
  connect(_reconnect_timer, SIGNAL(timeout()), this, SLOT(reconnect()));
  _reconnect_timer->setSingleShot(false);
  _reconnect_timer->start(5000);
}

DetectorSelect::~DetectorSelect() 
{
}


ConfigSelect::ConfigSelect(unsigned     interface,
			   const SList& servers,
			   const char*  db_path) :
  QGroupBox("Control"),
  _servers (servers),
  _expt    (Pds_ConfigDb::Path(db_path))
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
	s.write(&msg, sizeof(msg));
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
  l->addWidget(new DetectorSelect(interface, servers));
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


int main(int argc, char **argv) 
{
  unsigned interface   = 0x7f000001;
  const char* loadfile = 0;
  const char* configdb = 0;

  for(int i=0; i<argc; i++) {
    if (strcmp(argv[i],"-i")==0) {
      interface = parse_interface(argv[++i]);
    }
    else if (strcmp(argv[i],"-f")==0) {
      loadfile = argv[++i];
    }
    else if (strcmp(argv[i],"-c")==0) {
      configdb = argv[++i];
    }
  }

  QApplication app(argc, argv);

  SList servers;

  if (loadfile) {
    FILE* f = fopen(loadfile,"r");
    if (!f) {
      char buff[128];
      sprintf(buff,"Error opening file %s",loadfile);
      perror(buff);
    }

    size_t line_sz = 256;
    char*  line = (char *)malloc(line_sz);
    Ami::Blv::ServerEntry entry;
    unsigned det;

    while(getline(&line, &line_sz, f) != -1) {
      if (line[0]!='#') {
	if (sscanf(line,"%s\t%s\t%d\t%d",
		   entry.name, entry.host, &det, &entry.port) < 3)
	  fprintf(stderr,"Error scanning line: %s\n",line);
	else {
	  entry.info = Pds::DetInfo(0,(Pds::DetInfo::Detector)det,1,Pds::DetInfo::TM6740,0);
	  servers.push_back(entry);
	}
      }
    }
    if (line) {
      free(line);
    }
  }
    
  Control* control =
    new Control("Beamline Profile Monitoring",
		interface,servers,configdb);
  control->show();

  app.exec();

  return 0;
}
