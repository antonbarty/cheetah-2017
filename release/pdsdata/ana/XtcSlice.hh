#ifndef Pds_XtcSlice_hh
#define Pds_XtcSlice_hh

#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/ana/XtcPool.hh"
#include "pdsdata/index/IndexChunkReader.hh"

#include <list>
#include <string>

namespace Pds
{  
namespace Ana
{
enum Result { OK, End, Error };
  
extern bool _live;

class XtcSlice {
public:
  XtcSlice(std::string fname);
  ~XtcSlice();
public:
  bool add_file(std::string fname);
public:
  void   init();
  Result next(Pds::Dgram*& dg, int64_t* pi64OffsetCur = NULL);
  Result skip();
  Result jump(int calib, int jump, int& eventNum, bool bOverJump = false);

  Result findTime(uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime);  
  Result numCalib(int& iNumCalib);
  Result numEventInCalib(int calib, int& iNumEvents);
  Result getTime(int calib, int event, uint32_t& uSeconds, uint32_t& uNanoseconds);  
public:
  const Pds::Dgram& hdr() const { return *_nextdg; }
private:
  bool _open (int64_t i64Offset = 0);
  void _close(bool bForceWait = false);
  Result _openChunk(int iChunk, uint64_t i64Offset);
  Result _next();
  Result _loadIndex();
public:
  bool read();  
private:
  std::string _base;
  std::list<std::string> _chunks;
  std::list<std::string>::iterator _current;
  Pds::Dgram* _lastdg;
  Pds::Dgram* _nextdg;
  int _fd;
  pthread_attr_t _flags;
  pthread_t _threadID;
  XtcPool* _pool;
  volatile bool _bclose;
  int64_t                 _i64OffsetNext;
  Index::IndexChunkReader _index;   
};

} // namespace Ana
} // namespace Pds

#endif // #ifndef Pds_XtcSlice_hh
