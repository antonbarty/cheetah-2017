#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "XtcMonitorClient.hh"
#include "pdsdata/xtc/Dgram.hh"

class MyMonitorClient : public Pds::XtcMonitorClient {
public:
  MyMonitorClient(FILE* f)  : _f(f) {}
  int processDgram(Pds::Dgram* dg) {
    return fwrite(dg, sizeof(*dg)+dg->xtc.sizeofPayload(), 1, _f)==1 ? 0 : 1;
  }
private:
  FILE* _f;
};

void usage(char* progname) {
  fprintf(stderr,"Usage: %s -f <filename> [-p <partitionTag>] [-i <index>] [-h]\n", progname);
}


int main(int argc, char* argv[]) {
  int c;
  const char* partitionTag = 0;
  const char* fname = 0;
  unsigned index = 0;

  while ((c = getopt(argc, argv, "?hi:p:f:")) != -1) {
    switch (c) {
    case '?':
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'i':
      index = strtoul(optarg,NULL,0);
      break;
    case 'p':
      partitionTag = optarg;
      break;
    case 'f':
      fname = optarg;
      break;
    default:
      usage(argv[0]);
    }
  }

  if (!fname) {
    usage(argv[0]);
    return 1;
  }

  FILE* f = fopen(fname,"w");
  if (!f) {
    perror("Error opening output xtc file");
    return -1;
  }

  MyMonitorClient myClient(f);
  fprintf(stderr, "myClient returned: %d\n", myClient.run(partitionTag,index,index));

  fclose(f);

  return 1;
}
