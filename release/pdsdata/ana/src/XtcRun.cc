#include "pdsdata/ana/XtcRun.hh"

namespace Pds
{  
namespace Ana
{
bool _live=false;

void XtcRun::live_read(bool l) { _live=l; }

XtcRun::XtcRun() {}

XtcRun::~XtcRun() 
{
  for(std::list<XtcSlice*>::iterator it=_slices.begin();
      it!=_slices.end(); it++)
    delete (*it);
  _slices.clear();
} 

void XtcRun::reset   (std::string fname) 
{
  for(std::list<XtcSlice*>::iterator it=_slices.begin();
      it!=_slices.end(); it++)
    delete (*it);
  _slices.clear();
  _slices.push_back(new XtcSlice(fname));
  _base = fname.substr(0,fname.find("-s"));
}

bool XtcRun::add_file(std::string fname) 
{
  if (fname.compare(0,_base.size(),_base)!=0)
    return false;

  for(std::list<XtcSlice*>::iterator it=_slices.begin();
      it!=_slices.end(); it++)
    if ((*it)->add_file(fname))
      return true;
  _slices.push_back(new XtcSlice(fname));
  return true;
}

const char* XtcRun::base() const
{ return _base.c_str(); }

unsigned XtcRun::run_number() const 
{ return atoi(_base.c_str()+_base.find("-r")+2); }


void XtcRun::init() 
{
  for(std::list<XtcSlice*>::iterator it=_slices.begin();
      it!=_slices.end(); it++)
    (*it)->init();
}

Result XtcRun::next(Pds::Dgram*& dg, int* piSlice, int64_t* pi64OffsetCur)
{  
  //
  //  Process L1A with lowest clock time first
  //
  Pds::ClockTime tmin(-1,-1);
  
  int iSlice      = 0;
  int iNextSlice  = 0;
  std::list<XtcSlice*>::iterator n  = _slices.begin();
  for(std::list<XtcSlice*>::iterator it = _slices.begin();
      it != _slices.end(); it++, iSlice++) {
        
    if ((*it)->hdr().seq.service()==Pds::TransitionId::L1Accept &&
        tmin > (*it)->hdr().seq.clock())
    {
      tmin = (*(n = it))->hdr().seq.clock();
      iNextSlice  = iSlice;
    }      
  }

  //
  //  On a transition, advance all slices
  //
  if ((*n)->hdr().seq.service()!=Pds::TransitionId::L1Accept) {
    for(std::list<XtcSlice*>::iterator it = _slices.begin();
        it != _slices.end();) {
      if (it != n && (*it)->skip()==End) {
        delete (*it);
        std::list<XtcSlice*>::iterator ee = it++;
        _slices.erase(ee);
      }
      else
        it++;
    }
  }
  Result r = (*n)->next(dg, pi64OffsetCur);
  if (r == End) {
    delete (*n);
    _slices.erase(n);
    if (_slices.size())
      r = OK;
  }
  
  if (piSlice != NULL)
    *piSlice = iNextSlice;
      
  //printf("<%d>", iNextSlice);//!!debug
  return r;
}

int XtcRun::jump(int calib, int jump, int& eventNum)
{    
  if (jump == 0)
  {
    int iErrorAll     = 0;
    int iEventSum = 0;
    int iSlice        = 0;
    for(std::list<XtcSlice*>::iterator 
      it =  _slices.begin();
      it != _slices.end();
      it++, iSlice++) 
    {
      int iEventSlice;
      int iError = 
        (*it)->jump( calib, 0, iEventSlice );
                
      if ( iError == 0 )
        iEventSum += iEventSlice-1;
      else
        iErrorAll++;
    }
    
    if ( iErrorAll != 0 )
    {
      eventNum = -1;
      return 1;
    }
    
    ++iEventSum;      
    eventNum = iEventSum;
    return 0;
  }   
  
  struct TSliceJumpInfo
  {
    int iNumEvent;
    int iLowerBound;
    int iUpperBound;
    int iCurrentCalib;
    int iCurrentIndex;
  } infoList[ _slices.size() ];
  
  int iNumEventTotal = 0;
  int iSlice         = 0;
  for(std::list<XtcSlice*>::iterator 
    it =  _slices.begin();
    it != _slices.end();
    it++, iSlice++) 
  {
    infoList[iSlice].iLowerBound = 1;
    
    int iError = (*it)->numEventInCalib(calib, infoList[iSlice].iNumEvent);
    if (iError != 0)
    {
      printf("XtcRun::jump(): Cannot get Event# for Slice %d in Calib# %d\n", iSlice, calib);
      return 2;
    }
    
    infoList[iSlice].iUpperBound      = infoList[iSlice].iNumEvent;
    infoList[iSlice].iCurrentCalib    = -1;
    infoList[iSlice].iCurrentIndex    = -1;
    iNumEventTotal                 += infoList[iSlice].iNumEvent;
  }
  if (jump < 0 || jump > iNumEventTotal)
  {
    printf("XtcRun::jump(): Invalid Event# %d (Max # = %d)\n", jump, iNumEventTotal);
    return 3;
  }
  
  int iJumpAvg  = (int) ( (jump+_slices.size()-1) / _slices.size());   
  if (iJumpAvg < 1)  
    iJumpAvg = 1;
  else if (iJumpAvg > infoList[0].iUpperBound )
    iJumpAvg = infoList[0].iUpperBound;
  
  iSlice = 0;
  for(std::list<XtcSlice*>::iterator 
    it =  _slices.begin();
    it != _slices.end();
    it++, iSlice++) 
  {    
    int iTestIndex = ( ( iSlice == 0 ) ? iJumpAvg :
      (infoList[iSlice].iLowerBound + infoList[iSlice].iUpperBound)/2 );
    
    for (; infoList[iSlice].iLowerBound <= infoList[iSlice].iUpperBound;
      iTestIndex = (infoList[iSlice].iLowerBound + infoList[iSlice].iUpperBound)/2)
    {
      infoList[iSlice].iCurrentIndex = iTestIndex;
      
      uint32_t uSeconds, uNanoseconds;
      (*it)->getTime(calib, iTestIndex, uSeconds, uNanoseconds);
      
      int iLocalEventSum  = 0;
      int iSliceQuery     = 0;
      for(std::list<XtcSlice*>::iterator 
        itQuery =  _slices.begin();
        itQuery != _slices.end();
        itQuery++, iSliceQuery++) 
      {
        if (iSliceQuery == iSlice)
        {
          iLocalEventSum += iTestIndex-1;
          continue;
        }
        
        int   iCalibQuery       = -1;
        int   iEventQuery       = -1;
        bool  bExactMatchQuery  = false;
        bool  bOvertimeSlice    = false;
        int iError = (*itQuery)->findTime(uSeconds, uNanoseconds, iCalibQuery, iEventQuery, bExactMatchQuery, bOvertimeSlice);
        if (iError != 0)
        {
          if (bOvertimeSlice)
          {
            iCalibQuery = calib;
            iEventQuery = infoList[iSliceQuery].iNumEvent+1;
          }
          else
            continue;
        }
        
        if (iCalibQuery != calib)
        {
          if (iCalibQuery < calib)
          {
            printf("XtcRun::jump(): Inconsistent Calib# in different Slices: %d [Slice %d] and %d [Slice %d]\n",
              calib, iSlice, iCalibQuery, iSliceQuery );
            return 4;
          }
          else
          {
            iCalibQuery = calib;
            iEventQuery = infoList[iSliceQuery].iNumEvent+1;
          }                    
        }
        
        infoList[iSliceQuery].iCurrentIndex = iEventQuery;
        
        iLocalEventSum += iEventQuery-1;        
      } // itQuery loop
      
      ++iLocalEventSum;
      if (iLocalEventSum < jump)
      {
        infoList[iSlice].iLowerBound = infoList[iSlice].iCurrentIndex+1;
        for (int iSliceUpdate = iSlice+1; iSliceUpdate < (int) _slices.size(); iSliceUpdate++ )
        {
          if (infoList[iSliceUpdate].iCurrentIndex>infoList[iSliceUpdate].iLowerBound)
            infoList[iSliceUpdate].iLowerBound = infoList[iSliceUpdate].iCurrentIndex;
        }
      }
      else if (iLocalEventSum > jump)
      {
        infoList[iSlice].iUpperBound = infoList[iSlice].iCurrentIndex-1;
        for (int iSliceUpdate = iSlice+1; iSliceUpdate < (int) _slices.size(); iSliceUpdate++ )
        {
          if (infoList[iSliceUpdate].iCurrentIndex-1<infoList[iSliceUpdate].iUpperBound)
            infoList[iSliceUpdate].iUpperBound = infoList[iSliceUpdate].iCurrentIndex-1;
        }
      }
      else // (iLocalEventSum == jump)
      {  
        int iErrorAll = 0;
        
        int iGlobalEventSum = 0;
        int iSliceUpdate = 0;
        for(std::list<XtcSlice*>::iterator 
          itUpdate =  _slices.begin();
          itUpdate != _slices.end();
          itUpdate++, iSliceUpdate++)
        {
          int iEventSlice;          
          int iError = 
            (*itUpdate)->jump( calib, infoList[iSliceUpdate].iCurrentIndex, iEventSlice, true );
            
          if ( iError == 0 )
            iGlobalEventSum += iEventSlice-1;
          else
          {
            printf("XtcRun::jump(): Jump failed in Slice %d for Calib# %d Event# %d\n",
              iSliceUpdate, calib, infoList[iSliceUpdate].iCurrentIndex);
            iErrorAll++;
          }
        }

        if ( iErrorAll != 0 )
        {
          eventNum = -1;
          return 5;
        }
          
        ++iGlobalEventSum;
        eventNum = iGlobalEventSum; 
                
        printf("XtcRun::jump(): Jumped to Calib# %d Event# %d (Global# %d)\n",
          calib, iLocalEventSum, iGlobalEventSum); //!!debug            
        
        return 0;        
      } 
      
    } // binary search loop
  } // for each slice
  
  return 6;
}

int XtcRun::findTime(const char* sTime, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime)
{  
  uint32_t uSeconds = 0, uNanoseconds = 0;
  int iError = Index::convertTimeStringToSeconds( sTime, uSeconds, uNanoseconds );
 
  if (iError != 0)
    return 1;
  
  return findTime(uSeconds, uNanoseconds, iCalib, iEvent, bExactMatch, bOvertime);  
}

int XtcRun::findTime(uint32_t uSeconds, uint32_t uNanoseconds, int& iCalib, int& iEvent, bool& bExactMatch, bool& bOvertime)
{
  iCalib      = -1;
  iEvent      = 0;
  bExactMatch = false;
  
  int iErrorAll     = 0;
  int iOvertimeSum  = 0;
  int iCalibMin     = 0;
  
  struct TSliceFindTimeInfo
  {
    int iCalib;
    int iEvent;
  } infoList[ _slices.size() ];
  
  int iSlice = 0;
  for(std::list<XtcSlice*>::iterator 
    it =  _slices.begin();
    it != _slices.end();
    it++, iSlice++) 
  {
    int   iCalibSlice = -1, iEventSlice = -1;
    bool  bExactMatchSlice  = false;
    bool  bOvertimeSlice    = false;
    int iError = 
      (*it)->findTime( uSeconds, uNanoseconds, iCalibSlice, iEventSlice, bExactMatchSlice, bOvertimeSlice );
      
    if ( iError != 0 )
    {
      if ( bOvertimeSlice )
      {
        int iNumCalib = 0;
        int iError = (*it)->numCalib(iNumCalib);
        if (iError != 0)
        {
          printf("XtcRun::findTime(): Cannot get Calib# for Slice %d\n", iSlice);
          return 1;
        }                
        iCalibSlice = iNumCalib; // set current calib to be the last calib
        
        int iNumEventsInCalib = 0;
        iError = (*it)->numEventInCalib(iCalibSlice, iNumEventsInCalib);
        if (iError != 0)
        {
          printf("XtcRun::findTime(): Cannot get Event# for Slice %d in Calib# %d\n", iSlice, iCalibSlice);
          return 1;
        }           
        iEventSlice = iNumEventsInCalib+1; // set current event to be the last event+1
        
        printf("XtcRun::findTime(): Overtime for Slice %d, set Calib# %d Event# %d\n", 
          iSlice, iCalibSlice, iEventSlice );

        ++iOvertimeSum;
      }
      else
      {
        printf("XtcRun::findTime(): Failed in Slice %d\n", iSlice );
        iErrorAll++;
        continue;
      }
    }
    
    infoList[iSlice].iCalib = iCalibSlice;
    infoList[iSlice].iEvent = iEventSlice;
    
    if ( iSlice == 0 || iCalibSlice < iCalibMin )
      iCalibMin = iCalibSlice;
    
    if (bExactMatchSlice)
      bExactMatch = true;      
  }  
  
  if ( iErrorAll != 0 )  
    return 1;
    
  if (iOvertimeSum >= (int) _slices.size())
  {
    bOvertime = true;
    return 2;
  }
  
  int iEventSum = 0;
  iSlice = 0;
  for(std::list<XtcSlice*>::iterator 
    it =  _slices.begin();
    it != _slices.end();
    it++, iSlice++)   
  {
    if (infoList[iSlice].iCalib != iCalibMin)
    {
      int iNumEvent = -1;
      int iError = (*it)->numEventInCalib(iCalibMin, iNumEvent);
      if (iError != 0)
      {
        printf("XtcRun::findTime(): Cannot get Event# for Slice %d in Calib# %d\n", iSlice, iCalibMin);
        return 3;
      }
      infoList[iSlice].iEvent = iNumEvent+1;
    }    
    
    iEventSum += infoList[iSlice].iEvent-1;
  }
  
  iCalib = iCalibMin;
  iEvent = iEventSum + 1;
  return 0;
}

int XtcRun::numCalib(int& iNumCalib)
{
  if (_slices.size() == 0)
  {
    printf( "XtcRun::numCalib(): No slice found\n" );
    return 1;
  }    
  return (*_slices.begin())->numCalib(iNumCalib);
}

int XtcRun::numEventInCalib(int calib, int& iNumEvents)
{
  iNumEvents = -1;
  
  int iNumCalib = -1;  
  int iError    = numCalib(iNumCalib);    
  if ( iError != 0 )
  {
    printf( "XtcRun::numEventInCalib(): Query Calib# failed\n" );
    return 1;
  }
  
  if (calib < 1 || calib > iNumCalib)
  {
    printf( "XtcRun::numEventInCalib(): Invalid Calib# %d. Max # = %d\n", calib, iNumCalib );
    return 2;
  }
  
  int iEventSum = 0;
  
  int iSlice = 0;
  for(std::list<XtcSlice*>::iterator 
    it =  _slices.begin();
    it != _slices.end();
    it++, iSlice++) 
  {
    int iNumEvent = -1;
    iError = (*it)->numEventInCalib(calib, iNumEvent);
    if ( iError != 0 )
    {
      printf( "XtcRun::numEventInCalib(): Query Event# for Calib# %d in Slice %d failed\n", calib, iSlice );
      return 3;
    }
    
    iEventSum += iNumEvent;
  }
  
  iNumEvents = iEventSum;  
  return 0;
}

} // namespace Ana
} // namespace Pds

