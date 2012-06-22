#ifndef Pds_Index_IndexList_hh
#define Pds_Index_IndexList_hh

#include <vector>
#include <map>

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/Damage.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/Dgram.hh"

#include "IndexFileStruct.hh"

namespace Pds
{  
namespace Index
{
  
#pragma pack(1)
 
struct L1AcceptNode
{  
  uint32_t            uSeconds;
  uint32_t            uNanoseconds;
  uint32_t            uFiducial;
  int64_t             i64OffsetXtc;
  Damage              damage;
  uint32_t            uMaskDetDmgs;
  uint32_t            uMaskDetData;
  uint32_t            uMaskEvrEvents;
      
  L1AcceptNode();
  L1AcceptNode(uint32_t uSeconds1, uint32_t uNanoseconds1, uint32_t uFiducial1, int64_t i64Offset1);
  L1AcceptNode(IndexFileL1NodeType& fileNode);
  int laterThan(const L1AcceptNode& node);
    
  static const uint32_t uInvalidFiducial  = 0x1ffff;
  static const uint32_t uSegDmgNotPresent = 0x100000;
};

struct L1SegmentIndex
{
  ProcInfo procNode;
  
  L1SegmentIndex(const ProcInfo& procNode1);
  bool operator<(const L1SegmentIndex& right) const;
};

struct L1SegmentId
{
  typedef std::vector<Src>    TSrcList;
  typedef std::vector<TypeId> TTypeList;
  
  int       iIndex;
  TSrcList  srcList;
  TTypeList typeList;
  bool      bSrcUpdated;
  
  explicit L1SegmentId(int iIndex1);
};

class IndexList
{
public:
        IndexList  ();
        IndexList  (const char* sXtcFilename);        
        
  /*
   * L1Accept node operations
   */
  int   startNewNode  (const Dgram& dg, int64_t i64Offset, bool& bInvalidData);
  int   updateSegment (const Xtc& xtc);
  int   updateSource  (const Xtc& xtc, bool& bStopUpdate);
  int   updateReporter(const Xtc& xtc, bool& bStopUpdate);
  int   updateEvr     (const Xtc& xtc);
  int   finishNode    (bool bPrint);
  
  int   getNumNode    () const;
  int   getNode       (int iNode, L1AcceptNode*& pNode);
  
  /*
   * BeginCalibCycle operation
   */
  int   addCalibCycle (int64_t i64Offset, uint32_t uSeconds, uint32_t uNanoseconds);

  int   reset         (bool bClearL1NodeLast = false);    
  int   setXtcFilename(const char* sXtcFilename);
  int   finishList    ();  
  void  printList     (int iVerbose) const;  
  int   writeToFile   (int fdFile) const;  
  int   readFromFile  (int fdFile);  
  
private:    
  static const int iMaxFilenameLen  = IndexFileHeaderType::iMaxFilenameLen;

  typedef   std::vector<CalibNode>        TCalibList;  
  typedef   std::vector<L1AcceptNode>     TNodeList;  
  typedef   std::vector<Damage>           TSegmentDamageMapList;
  typedef   std::map<L1SegmentIndex,L1SegmentId>  
                                          TSegmentToIdMap;
  typedef   std::map<uint32_t,int>        TEvrEvtToIdMap;

  char                      _sXtcFilename[iMaxFilenameLen];  
  int                       _iNumSegments;
  TSegmentToIdMap           _mapSegToId;
  TCalibList                _lCalib;
  TEvrEvtToIdMap            _mapEvrToId;
  TSegmentToIdMap::iterator _itCurSeg;

  
  TNodeList                 _lNode;  
  bool                      _bNewNode;   
  L1AcceptNode*             _pCurNode;
  TSegmentDamageMapList     _lSegDmgTmp;
  
  int                       _iCurSerial;
  
  int                       _iNumOutOrder;
  bool                      _bOverlapChecked;
  L1AcceptNode              _l1NodeLast;
  int                       _iNumOverlapPrev;
  int                       _iNumOverlapNext;
  
  int           finishPrevSegmentId();
  L1AcceptNode& checkInNode( L1AcceptNode& nodeNew );  
  
  void          printNode(const L1AcceptNode& node, int iSerial) const;
  
  int           writeFileHeader       (int fdFile) const;
  int           writeFileL1AcceptList (int fdFile) const;
  int           writeFileSupplement   (int fdFile) const;

  int           readFileHeader        (int fdFile, IndexFileHeaderType& fileHeader);
  int           readFileL1AcceptList  (int fdFile, int iNumIndex);
  int           readFileSupplement    (int fdFile, const IndexFileHeaderType& fileHeader);
  
  friend class IndexFileHeaderV1;  
  friend class IndexFileHeaderV2;
}; // class IndexList

#pragma pack()

} // namespace Index
} // namespace Pds

#endif // #ifndef Pds_Index_IndexList_hh 
