#ifndef Pds_XtcRun_hh
#define Pds_XtcRun_hh

#include "pdsdata/ana/XtcSlice.hh"

#include <list>
#include <string>

namespace Pds
{  
namespace Ana
{
extern bool _live;

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
  Result next(Pds::Dgram*& dg, int* piSlice = NULL, int64_t* pi64OffsetCur = NULL);
public:
  int jump    (int calib, int jump, int& eventNum);
  int findTime(const char* sTime, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime);
  int findTime(uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime);
  int numCalib(int& iNumCalib);
  int numEventInCalib(int calib, int& iNumEvents);  
public:
  static void live_read(bool l);
  static void read_ahead(bool l);
private:
  std::list<XtcSlice*> _slices;
  std::string          _base;
};

} // namespace Ana
} // namespace Pds

#endif // #ifndef Pds_XtcRun_hh
