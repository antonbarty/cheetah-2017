#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "ami/qt/DetectorSelect.hh"
#include "ami/qt/Path.hh"
#include "ami/service/Ins.hh"

#include <QtGui/QApplication>

using namespace Ami;

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
  unsigned ppinterface = 0x7f000001;
  unsigned interface   = 0x7f000001;
  unsigned serverGroup = 0xefff2000;
  const char* loadfile = 0;

  for(int i=0; i<argc; i++) {
    if (strcmp(argv[i],"-I")==0) {
      ppinterface = parse_interface(argv[++i]);
    }
    else if (strcmp(argv[i],"-i")==0) {
      interface = parse_interface(argv[++i]);
    }
    else if (strcmp(argv[i],"-s")==0) {
      in_addr inp;
      if (inet_aton(argv[++i], &inp))
	serverGroup = ntohl(inp.s_addr);
    }
    else if (strcmp(argv[i],"-f")==0) {
      Ami::Qt::Path::setBase(argv[++i]);
    }
    else if (strcmp(argv[i],"-F")==0) {
      loadfile = argv[++i];
    }
  }

  QApplication app(argc, argv);

  Ami::Qt::DetectorSelect* select = new Ami::Qt::DetectorSelect("DAQ Online Monitoring",
								ppinterface,interface,serverGroup);
  select->show();

  if (loadfile) {
    FILE* f = fopen(loadfile,"r");
    if (f) {
      const int MaxConfigSize = 0x100000;
      char* buffer = new char[MaxConfigSize];
      int size = fread(buffer,1,MaxConfigSize,f);
      fclose(f);
      select->set_setup(buffer,size);
      delete[] buffer;
    }
    else {
      printf("Unable to open %s\n",loadfile);
    }
  }

  app.exec();

  return 0;
}
