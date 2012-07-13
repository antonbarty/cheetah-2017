#include "ami/qt/DetectorSelect.hh"
#include "ami/qt/Path.hh"
#include "ami/qt/ImageColorControl.hh"
#include "ami/service/Ins.hh"

#include <QtGui/QApplication>

static void usage(char* p)
{
  printf("Usage: %s -I <interface> [-i <interface>] -s <address> [-f <path> -F <path> -C <int>]\n" \
         "arguments: <interface> = IP address (dot notation) or ethX\n" \
         "           <address>   = IP address (dot notation)\n" \
         "           <path>      = full file path\n" \
         "-I <interface> : point-to-point interface (cds subnet)\n" \
         "-i <interface> : multicast interface (fez subnet), reqd if -s is multicast group\n" \
         "-s <address>   : server multicast group or proxy address\n" \
         "-f <path>      : default path for load/save operations\n" \
         "-F <path>      : file to load initial configuration\n" \
         "-C <int>       : color palette choice (0-jette, 1-radiation)\n", p);
}

int main(int argc, char **argv) 
{
  unsigned ppinterface = 0x7f000001;
  unsigned interface   = 0x7f000001;
  unsigned serverGroup = 0xefff2000;
  const char* loadfile = 0;

  for(int i=0; i<argc; i++) {
    if (strcmp(argv[i],"-I")==0) {
      ppinterface = Ami::Ins::parse_interface(argv[++i]);
    }
    else if (strcmp(argv[i],"-i")==0) {
      interface = Ami::Ins::parse_interface(argv[++i]);
    }
    else if (strcmp(argv[i],"-s")==0) {
      serverGroup = Ami::Ins::parse_ip(argv[++i]);
    }
    else if (strcmp(argv[i],"-f")==0) {
      Ami::Qt::Path::setBase(argv[++i]);
    }
    else if (strcmp(argv[i],"-F")==0) {
      loadfile = argv[++i];
    }
    else if (strcmp(argv[i],"-C")==0) {
      Ami::Qt::ImageColorControl::set_color_choice(atoi(argv[++i]));
    }
    else if (strcmp(argv[i],"-h")==0 ||
             strcmp(argv[i],"-?")==0) {
      usage(argv[0]);
      exit(1);
    }
  }
  QApplication app(argc, argv);
  Ami::Qt::DetectorSelect* select = new Ami::Qt::DetectorSelect("DAQ Online Monitoring", ppinterface, interface, serverGroup);
  select->show();
  if (loadfile) {
    select->load_setup(loadfile);
  }
  app.exec();
}
