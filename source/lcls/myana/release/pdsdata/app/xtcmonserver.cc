#include <iostream>
#include <signal.h>
#include "XtcRunSet.hh"

using namespace std;

static void sigfunc(int sig_no) {
  cout << endl << "Caught " << strsignal(sig_no) << " signal, exiting..." << endl;
  exit(0);
}

void usage(char* progname) {
  cerr << "Usage: " << progname
       << " (-f <filename> | -l <filename_list> | -x <run_file_prefix> | -d <xtc_dir>)" // choose one
       << " -p <partitionTag> -n <numberOfBuffers> -s <sizeOfBuffers>" // mandatory
       << " [-r <ratePerSec>] [-c <# clients>] [-S <sequence length>]" // optional
       << " [-L] [-v] [-V]" // debugging (optional)
       << endl;
}

int main(int argc, char* argv[]) {
  // Exactly one of these values must be supplied
  char* xtcFile = NULL;
  char* listFile = NULL;
  char* runPrefix = NULL;
  char* xtcDir = NULL;

  // These are mandatory
  int numberOfBuffers = 4;
  unsigned sizeOfBuffers = 0x900000;
  char* partitionTag = NULL;

  // These are optional
  int rate = 60; // Hz
  unsigned nclients = 1;
  unsigned sequenceLength = 1;

  // These are for debugging (also optional)
  bool loop = false;
  bool verbose = false;
  bool veryverbose = false;

  //  (void) signal(SIGINT, sigfunc);
  //  (void) signal(SIGSEGV, sigfunc);

  int c;
  while ((c = getopt(argc, argv, "f:l:x:d:p:n:s:r:c:S:LvVh?")) != -1) {
    switch (c) {
      case 'f':
        xtcFile = optarg;
        break;
      case 'l':
        listFile = optarg;
        break;
      case 'x':
        runPrefix = optarg;
        break;
      case 'd':
        xtcDir = optarg;
        break;
      case 'n':
        sscanf(optarg, "%d", &numberOfBuffers);
        break;
      case 's':
        sizeOfBuffers = (unsigned) strtoul(optarg, NULL, 0);
        break;
      case 'r':
        sscanf(optarg, "%d", &rate);
        break;
      case 'p':
        partitionTag = optarg;
        break;
      case 'c':
        nclients = strtoul(optarg, NULL, 0);
        break;
      case 'S':
        sscanf(optarg, "%d", &sequenceLength);
        break;
      case 'L':
        loop = true;
        printf("Enabling infinite looping\n");
        break;
      case 'v':
        verbose = true;
        break;
      case 'V':
        verbose = true;
        veryverbose = true;
        break;
      case 'h':
      case '?':
        usage(argv[0]);
        exit(0);
      default:
        fprintf(stderr, "Unrecognized option -%c!\n", c);
        usage(argv[0]);
        exit(0);
    }
  }

  if (sizeOfBuffers == 0 || numberOfBuffers == 0) {
    cerr << "Must specify both size (-s) and number (-n) of buffers." << endl;
    usage(argv[0]);
    exit(2);
  }
  if (partitionTag == NULL) {
    partitionTag = getenv("USER");
    if (partitionTag == NULL) {
      cerr << "Must specify a partition tag." << endl;
      usage(argv[0]);
      exit(2);
    }
  }
  int choiceCount = (xtcFile != NULL) + (listFile != NULL) + (runPrefix != NULL) + (xtcDir != NULL);
  if (choiceCount != 1) {
    cerr << "Must specify exactly one of -f, -l, -r, -d. You specified " << choiceCount << endl;
    usage(argv[0]);
    exit(2);
  }

  XtcRunSet runSet;
  runSet.connect(partitionTag, sizeOfBuffers, numberOfBuffers, nclients, sequenceLength, rate, verbose, veryverbose);
  do {
    if (xtcFile) {
      runSet.addSinglePath(xtcFile);
    } else if (xtcDir) {
      runSet.addPathsFromDir(xtcDir);
    } else if (listFile) {
      runSet.addPathsFromListFile(listFile);
    } else {
      runSet.addPathsFromRunPrefix(runPrefix);
    }
    runSet.run();
  } while (loop);
}
