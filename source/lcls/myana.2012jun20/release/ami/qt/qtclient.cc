#include "ami/qt/DetectorSelect.hh"
#include "ami/qt/Path.hh"
#include "ami/service/Ins.hh"

#include <QtGui/QApplication>

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
  }
  QApplication app(argc, argv);
  Ami::Qt::DetectorSelect* select = new Ami::Qt::DetectorSelect("DAQ Online Monitoring", ppinterface, interface, serverGroup);
  select->show();
  if (loadfile) {
    select->load_setup(loadfile);
  }
  app.exec();
}
