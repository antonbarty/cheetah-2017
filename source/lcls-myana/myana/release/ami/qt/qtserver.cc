//
//  Fetch parameters for starting xtcmonserver (file path) and ami (skim filter/output)
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "ami/qt/Client.hh"
#include "ami/client/VClientManager.hh"

#include "ami/service/Ins.hh"

#include <QtGui/QApplication>

using namespace Ami;

typedef Pds::DetInfo DI;

int main(int argc, char **argv) 
{
  unsigned interface = 0;
  unsigned serverGroup = 0xefff2000;
  DI src(0,DI::NoDetector,0,DI::Acqiris,0);

  for(int i=0; i<argc; i++) {
    if (strcmp(argv[i],"-i")==0) {
      in_addr inp;
      if (inet_aton(argv[++i], &inp))
	interface = ntohl(inp.s_addr);
    }
    if (strcmp(argv[i],"-s")==0) {
      in_addr inp;
      if (inet_aton(argv[++i], &inp))
	serverGroup = ntohl(inp.s_addr);
    }
    else if (strcmp(argv[i],"-d")==0) {
      src = DI(0,DI::Detector(strtoul(argv[++i],0,0)),0,DI::Acqiris,0);
    }
  }
      
  QApplication app(argc, argv);

  Ami::Qt::Client* client = new Ami::Qt::Client(src,0);
  VClientManager manager(interface, serverGroup, *client);
  client->managed(manager);

  manager.connect();

  app.exec();

//   client.wait();

//   manager.configure();
//   client.wait();

  manager.disconnect();

  return 0;
}
