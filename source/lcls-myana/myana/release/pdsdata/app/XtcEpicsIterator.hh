#ifndef XTC_EPICS_ITERATOR_H
#define XTC_EPICS_ITERATOR_H

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/epics/EpicsPvData.hh"

namespace Pds
{
    
class XtcEpicsIterator : 
  public XtcIterator 
{
public:
    XtcEpicsIterator(Xtc* xtc, unsigned int iDepth) : XtcIterator(xtc), _iDepth(iDepth) {}
    virtual int process(Xtc* xtc);
    
    static const int iXtcVersion = 1;    
    static const Src srcLevel;
    static const int iMaxXtcSize = sizeof(EpicsPvCtrl<DBR_DOUBLE>) * 2600; // Space enough for 2000+ PVs of type DBR_DOUBLE
    static const TypeId::Type typeIdXtc = TypeId::Id_Epics;    
private:
    unsigned int _iDepth;
};    
    
} // namespace Pds 

#endif
