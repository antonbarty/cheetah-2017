#ifndef Pds_XtcRunSet_hh
#define Pds_XtcRunSet_hh

#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/ana/XtcRun.hh"

using std::string;
using std::list;
using Pds::Dgram;
using Pds::Ana::XtcRun;

class XtcRunSet {
private:
  list<string> _paths;
  XtcRun _run;
  bool _runIsValid;
  class MyMonitorServer* _server;
  long long int _period;
  bool _verbose;
  bool _veryverbose;
  bool _skipToNextRun();
  void _addPaths(list<string> newPaths);
  long long int timeDiff(struct timespec* end, struct timespec* start);
  Dgram* next();

public:
  XtcRunSet();
  void addSinglePath(string path);
  void addPathsFromDir(string dirPath, string matchString = "");
  void addPathsFromRunPrefix(string runPrefix);
  void addPathsFromListFile(string listFile);
  void connect(char* partitionTag, unsigned sizeOfBuffers, int numberOfBuffers, unsigned nclients, unsigned sequenceLength, int rate, bool verbose = false, bool veryverbose = false);
  void run();
};

#endif
