#ifndef Pds_Index_XtcIterL1Accept_hh
#define Pds_Index_XtcIterL1Accept_hh

#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/index/IndexList.hh"

namespace Pds
{  
namespace Index
{
  
class XtcIterL1Accept: public XtcIterator
{
public:
  enum
  { Stop, Continue };
  
  XtcIterL1Accept(Xtc * xtc, unsigned depth, int64_t i64Offset, 
    IndexList& indexList ) :
    XtcIterator(xtc), _depth(depth), _i64Offset(i64Offset), 
    _pIndexList(&indexList)
  {
  }

  int  process(Xtc * xtc);
  
private:
  unsigned            _depth;
  int64_t             _i64Offset;
  IndexList*          _pIndexList;
};

} // namespace Index
} // namespace Pds

#endif // #ifndef Pds_Index_XtcIterL1Accept_hh
