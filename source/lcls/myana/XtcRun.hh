#ifndef XtcRun_hh
#define XtcRun_hh

#include <pdsdata/xtc/Dgram.hh>

#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//
//  Some C++ classes to manage slice/chunk complexities
//
enum Result { OK, End, Error };

class XtcSlice {
public:
  XtcSlice(std::string fname);
  ~XtcSlice();
public:
  bool add_file(std::string fname);
public:
  void   init();
  Result next(Pds::Dgram* dg);
  Result skip();
public:
  const Pds::Dgram& hdr() const { return _hdr; }
private:
  bool _open();
  Result _next();
  Result _read(void* buf, ssize_t insz, bool seekNewChunk);
public:
  static void live_read(bool l);
private:
  std::string _base;
  std::list<std::string> _chunks;
  std::list<std::string>::iterator _current;
  Pds::Dgram _hdr;
  int _fd;
};


class XtcRun {
public:
  XtcRun();
  ~XtcRun();
public:
  void reset   (std::string fname);
  bool add_file(std::string fname);
public:
  const char* base() const;
  unsigned    run_number() const;
public:
  void   init();
  Result next(Pds::Dgram* dg);
private:
  std::list<XtcSlice*> _slices;
  std::string          _base;
};


#endif
