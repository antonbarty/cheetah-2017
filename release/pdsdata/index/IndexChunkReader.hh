#ifndef Pds_IndexChunkReader_hh
#define Pds_IndexChunkReader_hh

#include <vector>
#include <string>

#include "pdsdata/index/IndexFileReader.hh"

namespace Pds
{  
namespace Index
{

class IndexFilereader;

class IndexChunkReader
{
public:
      IndexChunkReader  ();
      ~IndexChunkReader ();  
  
  int   open          (const char* sFnXtc);
  int   close         ();
  bool  isValid       () const; 

  int numL1Event        (int& iNumL1Event) const;
  int detectorList      (int& iNumDetector, const ProcInfo*& lDetector) const;
  int srcList           (int iDetector, int& iNumSrc, const Src*& lSrc) const;
  int typeList          (int iDetector, int& iNumType, const TypeId*& lType) const;
  int numCalibCycle     (int& iNumCalib) const;
  int numL1EventInCalib (int iCalib, int& iNumL1Event) const;
  int eventCalibToGlobal(int iCalib, int iEvent, int& iGlobalEvent) const;
  int eventGlobalToCalib(int iGlobalEvent, int& iCalib, int& iEvent) const;
  int eventChunkToGlobal(int iChunk, int iEvent, int& iGlobalEvent) const;
  int eventGlobalToChunk(int iGlobalEvent, int& iChunk, int& iEvent) const;
  int eventTimeToGlobal (uint32_t uSeconds, uint32_t uNanoseconds, int& iGlobalEvent, bool& bExactMatch, bool& bOvertime) const;
  int eventTimeToCalib  (uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime) const;
  int eventTimeToChunk  (uint32_t uSeconds, uint32_t uNanoseconds, int& iChunk, int& iEvent, bool& bExactMatch, bool& bOvertime) const;
  
  int gotoEvent         (int iCalib, int iEvent, int& iChunk, int64_t& i64Offset, int& iGlobalEvent);
  
  int time            (int iGlobalEvent, uint32_t& uSeconds, uint32_t& uNanoseconds);
  int fiducial        (int iGlobalEvent, uint32_t& uFiducial);
  int offset          (int iGlobalEvent, int64_t&  i64Offset);
  int damage          (int iGlobalEvent, Damage& damage);
  int detDmgMask      (int iGlobalEvent, uint32_t& uMaskDetDmgs);
  int detDataMask     (int iGlobalEvent, uint32_t& uMaskDetData);
  int evrEventList    (int iGlobalEvent, unsigned int& uNumEvent, const uint8_t*& lEvrEvent);

private:
  int _buildInfo          ();
  int _addEventToPrevCalib(int iGlobalStartIndex, int iChunk, int iNumCalibEvent);
  int _makeNewCalib       (int iGlobalStartIndex, int iChunk, int iCalib, int iNumCalibEvent, int iChunkStartIndex, int64_t i64ChunkOffset);
  int _listCalib          ();

  struct ChunkCalibNode
  {
    int32_t iGlobalL1Index;   // index number of the first L1Accept event in all Calib Cycles and all chunks
    int     iChunk;           // Chunk number for this calib node
    int     iChunkCalib;      // Calib cycle # in the current chunk
    int32_t iCalibL1Index;    // index number of the first L1Accept event in this Calib Cycle (which may be contained in multiple chunks)
    int     iChunkNumL1Event; // number of L1 events in this Calib
    
    ChunkCalibNode() {}
  };  

  typedef std::vector<ChunkCalibNode>   TListChunkCalib;
  struct CalibInfo
  { 
    int             iNumL1Event;
    int32_t         iChunkL1Index;    // index number of the first L1Accept event in this Calib Cycle, in the current chunk
    int64_t         i64ChunkOffset;   // offset in the xtc file for jumping to this BeginCalibCycle
    TListChunkCalib lChunkCalib;
    
    CalibInfo() {}
  };
    
  typedef std::vector<IndexFileReader*> TListIndex;
  typedef std::vector<CalibInfo>        TListCalib;
  
  bool        _bValid;
  std::string _strFnBase;    
  TListIndex  _lIndex;   
  TListCalib  _lCalib;
              
  
  int         _iNumTotalL1Event;
};
  
}//namespace Index
}//namespace Pds
 

#endif //#ifndef Pds_IndexChunkReader_hh
