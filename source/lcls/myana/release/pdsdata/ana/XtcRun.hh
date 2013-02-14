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

  void reset   (std::string fname);
  bool add_file(std::string fname);

  const char* base() const;
  unsigned    run_number() const;

  void   init();
  Result next(Pds::Dgram*& dg, int* piSlice = NULL, int64_t* pi64OffsetCur = NULL);

  int jump    (int calib, int jump, int& eventNum);
 
  int findTime(const char* sTime, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime);
  int findTime(uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime);
  int getStartAndEndTime(ClockTime& start, ClockTime& end);
  int findNextFiducial
              (uint32_t uFiducialSearch, int iFidFromEvent, int& iCalib, int& iEvent);
  int numCalib(int& iNumCalib);
  int numEventInCalib(int calib, int& iNumEvents);  

  static void live_read(bool l);
  static void read_ahead(bool l);
  
private:
  int eventGlobalToSlice
              ( int iGlobalEvent, std::vector<int>& lSliceEvent );

  std::list<XtcSlice*> _slices;
  std::list<XtcSlice*> _doneSlices;
  std::string          _base;
  bool _startAndEndValid;
  ClockTime _start;
  ClockTime _end;
};

} // namespace Ana
} // namespace Pds

#endif // #ifndef Pds_XtcRun_hh
