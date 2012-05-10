#include "ami/app/SummaryAnalysis.hh"
#include "ami/app/SyncAnalysis.hh"

#include "ami/data/Cds.hh"
#include "ami/data/EntryScalar.hh"
#include "ami/data/DescScalar.hh"

#define DARK_SHOT_EVENT_CODE    162
#define DATA_RECORD_POINTS      10000,1000       //LiteShots, DarkShots points 
#define DISPLAY_N_POINTS        8000
#define REFILL_AFTER_POINTS     1000 
#define MIN_MAX_MARGIN_PERCENT  10
#define NO_OF_BINS              500
#define X_MIN                   0
#define X_MAX                   100
#define AUTO_DETECTION_ENB      false
#define H2D_DET_X_DATATYPE      Pds::TypeId::Id_EBeam
#define H2D_DET_Y_DATATYPE      Pds::TypeId::Id_PhaseCavity

//#define VERBOSE

using namespace Ami;

static EntryScalar* _seconds = 0;
static EntryScalar* _minutes = 0;  
static Cds* _cds = 0;
static Pds::ClockTime _clk;

SummaryAnalysis::SummaryAnalysis():_darkShot(false),_summaryEntries(0),_analyzeCount(0),_notRefilledCount(0) { }
SummaryAnalysis::~SummaryAnalysis() 
{
  if (!_summaryEntryEList.empty())
    _summaryEntryEList.clear();

  if (!_syncAnalysisPList.empty())
    _syncAnalysisPList.clear(); 

  if (_scatterPlotEntry) {  
    delete _scatterPlotEntry;
    _scatterPlotEntry = 0;
  }
 
}

void SummaryAnalysis::reset () 
{ 
#ifdef VERBOSE
  printf("SummaryAnalysis::reset \n");
#endif
  if (!_syncAnalysisPList.empty())
    _syncAnalysisPList.clear();  
  _summaryEntries = 0;  

}

void SummaryAnalysis::clock (const Pds::ClockTime& clk) 
{ 
  //printf("SummaryAnalysis::clk \n"); 
  _clk=clk;
}

void SummaryAnalysis::configure ( const Pds::Src& src, const Pds::TypeId& type, void* payload)
{
#ifdef VERBOSE
  printf("SummaryAnalysis::Configure  %s \n", Pds::TypeId::name(type.id()));
#endif
  SyncAnalysis* h = 0;  
  const DetInfo& detInfo = reinterpret_cast<const Pds::DetInfo&>(src); 

  switch(type.id())  {
    case Pds::TypeId::Id_AcqConfig:
      h = new acqDataSpace(detInfo, Pds::TypeId::Id_AcqWaveform,      Pds::TypeId::Id_AcqConfig,       payload, "Acqiris");
      break;
    case Pds::TypeId::Id_Opal1kConfig:
      h = new opalDataSpace(detInfo, Pds::TypeId::Id_Frame,           Pds::TypeId::Id_Opal1kConfig,    payload, "Opal");
      break; 
    case Pds::TypeId::Id_FccdConfig:
      h = new fccdDataSpace(detInfo, Pds::TypeId::Id_Frame,           Pds::TypeId::Id_FccdConfig,      payload, "Fccd"); 
      break; 
    case Pds::TypeId::Id_TM6740Config:
      h = new pulnixDataSpace(detInfo, Pds::TypeId::Id_Frame,              Pds::TypeId::Id_TM6740Config,    payload, "Pulnix");
      break; 
    case Pds::TypeId::Id_PrincetonConfig:
      h = new princetonDataSpace(detInfo, Pds::TypeId::Id_PrincetonFrame,  Pds::TypeId::Id_PrincetonConfig, payload, "Princeton");
      break;
    case Pds::TypeId::Id_pnCCDconfig:
      h = new pnccdDataSpace(detInfo, Pds::TypeId::Id_pnCCDframe,          Pds::TypeId::Id_pnCCDconfig,     payload, "PnCCD");
      break;   
    case Pds::TypeId::Id_IpimbConfig:
      h = new ipimbDataSpace(detInfo, Pds::TypeId::Id_IpimbData,           Pds::TypeId::Id_IpimbConfig,     payload, "IPIMB");  
      break;
    case Pds::TypeId::Id_FEEGasDetEnergy:
      h = new gasDetectorDataSpace(detInfo, Pds::TypeId::Id_FEEGasDetEnergy, Pds::TypeId::Id_FEEGasDetEnergy, payload, "GasDetector");
      break; 
    case Pds::TypeId::Id_EBeam:
      h = new eBeamDataSpace(detInfo, Pds::TypeId::Id_EBeam,                Pds::TypeId::Id_EBeam,           payload, "EBeam");       
      break;
    case Pds::TypeId::Id_PhaseCavity:
      h = new phaseCavityDataSpace(detInfo, Pds::TypeId::Id_PhaseCavity,    Pds::TypeId::Id_PhaseCavity,     payload, "PhaseCavity"); 
      break;

    default: break;
  } 
  if (h) {
    //printf("SummaryAnalysis::received configType: %s \n", Pds::TypeId::name(type.id()));
    insert(h);
#ifdef VERBOSE
    printf("%d th Summary Entry made: %s/%d/%s/%d \n",_summaryEntries, detInfo.name(detInfo.detector()),
      detInfo.detId(), detInfo.name(detInfo.device()),detInfo.devId());  
#endif
    _summaryEntries++;
  }
}
  

void SummaryAnalysis::event (const Pds::Src& src, const Pds::TypeId& type, void* payload)
{
  static bool throttle=false, throttle2=false;
  const DetInfo& detInfo = reinterpret_cast<const Pds::DetInfo&>(src); 
  if (type.id() == Pds::TypeId::Id_EvrData) 
    _evrEventData = const_cast<Pds::EvrData::DataV3*>(reinterpret_cast<const Pds::EvrData::DataV3*>(payload));
  else if((type.id() == Pds::TypeId::Id_AcqWaveform)     || (type.id() == Pds::TypeId::Id_Frame) || 
          (type.id() == Pds::TypeId::Id_FEEGasDetEnergy) || (type.id() == Pds::TypeId::Id_EBeam) ||
          (type.id() == Pds::TypeId::Id_PhaseCavity)     || (type.id() == Pds::TypeId::Id_PrincetonFrame) ||
          (type.id() == Pds::TypeId::Id_pnCCDframe)      || (type.id() == Pds::TypeId::Id_IpimbData)  ) {
    if (_syncAnalysisPList.empty()) {
      if (!throttle) {
#ifdef VERBOSE
	printf("**** ERROR:: event() called while SyncEntry List is Empty \n");
#endif
	throttle=true;
      }
    }
    else {
      throttle=false;
      bool entryFoundFlag = false;   
      for(PList::iterator it = _syncAnalysisPList.begin(); it != _syncAnalysisPList.end(); it++) {
        SyncAnalysis* syncPtr = *it;
        if ((detInfo.detector() == syncPtr->detInfo.detector()) &&
            (detInfo.detId()    == syncPtr->detInfo.detId())    &&
            (detInfo.device()   == syncPtr->detInfo.device())   &&
            (detInfo.devId()    == syncPtr->detInfo.devId()) ) {
          syncPtr->logEventDataPayload(payload); 
          entryFoundFlag = true;
          break;        
        }
      }
      if(!entryFoundFlag) {
	if (!throttle2) {
#ifdef VERBOSE
	  printf("**** ERROR:: event() called & no Entry Present in List for given detector \n");
#endif
	  throttle2=true;
	}
      }
    }  
  }

}

void SummaryAnalysis::clear () 
{
#ifdef VERBOSE
  printf("SummaryAnalysis::clear\n");
#endif
  if (_seconds) 
    _cds->remove(_seconds);     
  if(_minutes)
    _cds->remove(_minutes);

  for(EList::iterator itSummary = _summaryEntryEList.begin(); itSummary != _summaryEntryEList.end(); itSummary++) {
    EntryTH1F* summaryEntry = *itSummary;
    if(summaryEntry)
      _cds->remove(summaryEntry);
  }
  if (!_summaryEntryEList.empty())
    _summaryEntryEList.clear();

  if (_scatterPlotEntry) { 
    _cds->remove(_scatterPlotEntry); 
    delete _scatterPlotEntry;
    _scatterPlotEntry = 0;
  }

   _cds = 0;

}


void SummaryAnalysis::create   (Cds& cds)
{
#ifdef VERBOSE
  printf("SummaryAnalysis::create\n");
#endif
  _notRefilledCount = 0;
  _analyzeCount = 0;
  _scatterPlotBinsCount = 0;

  _seconds = new EntryScalar(DescScalar("Seconds#Time","Seconds"));
  _minutes = new EntryScalar(DescScalar("Minutes#Time","Minutes"));
  
  _cds = &cds; 
  cds.add(_seconds);
  cds.add(_minutes);

  //check presence of 2 detectors
  _detXYPresent = false;
  _syncPtrDetX = 0;
  _syncPtrDetY = 0;
  char displayTitle[50]; 
  EntryTH1F* summaryEntry = 0;
  _scatterPlotEntry= 0;

  for(PList::iterator it = _syncAnalysisPList.begin(); it != _syncAnalysisPList.end(); it++) {
    SyncAnalysis* syncPtr = *it;
    if (syncPtr->getDataType() == H2D_DET_X_DATATYPE)   _syncPtrDetX = *it;
    if (syncPtr->getDataType() == H2D_DET_Y_DATATYPE)   _syncPtrDetY = *it;
  }

  //for 2-D correleation plot
  if ( (_syncPtrDetX != 0) && (_syncPtrDetY != 0) ) {				
    sprintf(displayTitle,"%s/%s#Ebeam-PhCvty",_syncPtrDetY->getTitle(),_syncPtrDetX->getTitle());
    _scatterPlotEntry = new EntryScan(_syncPtrDetX->detInfo, 0, displayTitle,_syncPtrDetX->getTitle(),_syncPtrDetY->getTitle());
    _scatterPlotEntry->params(DISPLAY_N_POINTS); 
    cds.add(_scatterPlotEntry);
    _detXYPresent = true;
  } 

  //for 1-D Histogram plot
  for(PList::iterator it = _syncAnalysisPList.begin(); it != _syncAnalysisPList.end(); it++) {
    SyncAnalysis* syncPtr = *it;
    if (syncPtr->arrayBuiltFlag())
      syncPtr->syncReset();                     // reset stored Data Buffers  
    else 
      syncPtr->buildArray(DATA_RECORD_POINTS);  // build Data Buffers 

    Pds::TypeId::Type  dataType = syncPtr->getDataType();
    // Lite Data Plots Entry
    if( (dataType == Pds::TypeId::Id_FEEGasDetEnergy) || (dataType == Pds::TypeId::Id_EBeam) || (dataType == Pds::TypeId::Id_PhaseCavity) )
      sprintf(displayTitle,"LightShotsSummary#%s",syncPtr->getTitle());
    else
      sprintf(displayTitle,"LightShotsSummary#%s/%d/%s/%d",syncPtr->detInfo.name(syncPtr->detInfo.detector()), syncPtr->detInfo.detId(),
        syncPtr->detInfo.name(syncPtr->detInfo.device()),syncPtr->detInfo.devId());
    summaryEntry = new EntryTH1F(syncPtr->detInfo, 0, displayTitle,"Range","Value");
    summaryEntry->params(NO_OF_BINS, X_MIN, X_MAX);
    insertEntry(summaryEntry);
    cds.add(summaryEntry); 
 
    //Dark Data Plots Entry
    if( (dataType == Pds::TypeId::Id_FEEGasDetEnergy) || (dataType == Pds::TypeId::Id_EBeam) || (dataType == Pds::TypeId::Id_PhaseCavity) )
      sprintf(displayTitle,"DarkShotsSummary#%s",syncPtr->getTitle());
    else
      sprintf(displayTitle,"DarkShotsSummary#%s/%d/%s/%d",syncPtr->detInfo.name(syncPtr->detInfo.detector()), syncPtr->detInfo.detId(),
        syncPtr->detInfo.name(syncPtr->detInfo.device()),syncPtr->detInfo.devId());  
    summaryEntry = new EntryTH1F(syncPtr->detInfo, 0, displayTitle,"Range","Value");
    summaryEntry->params(NO_OF_BINS, X_MIN, X_MAX);
    insertEntry(summaryEntry);
    cds.add(summaryEntry);     
  }
}

void SummaryAnalysis::analyze  () 
{
  //printf("SummaryAnalysis::analyze \n");
  if (_cds) {
    // EVR Data Read for darkShot check
    for(unsigned i=0;i < _evrEventData->numFifoEvents(); i++) {
      if ( _evrEventData->fifoEvent(i).EventCode == DARK_SHOT_EVENT_CODE) { 
        _darkShot = true;
   	printf("Dark Shot Found \n");
        break;
      } else  _darkShot = false;      
    }

    // Process all detector's data
    double val = 0;
    for(PList::iterator it = _syncAnalysisPList.begin(); it != _syncAnalysisPList.end(); it++) {
      SyncAnalysis* syncPtr = *it;
      if (syncPtr->newEvent()) {
        val = syncPtr->processData();
        syncPtr->logDataPoint(val, _darkShot);
        syncPtr->setNewEventFlag(false);
      }
    }
      
    //Update Plot Data
    _plot2DRefill = false; 
    PList::iterator it = _syncAnalysisPList.begin();
    for(EList::iterator itSummary = _summaryEntryEList.begin(); itSummary != _summaryEntryEList.end(); itSummary++, it++) {
      SyncAnalysis* syncPtr = *it;
      EntryTH1F* summaryEntry = *itSummary;
      itSummary++;
      if (_darkShot) 
        refillPlotData(syncPtr, summaryEntry, *itSummary, DISPLAY_N_POINTS, ForceRefill) ; 
      else if(_notRefilledCount >= DISPLAY_N_POINTS)
        refillPlotData(syncPtr, summaryEntry, *itSummary, DISPLAY_N_POINTS, ForceRefill) ;      
      else {
        // initially for 100 events, check refill for every 5th event 
        if((_analyzeCount < 100) && ((_analyzeCount % 5)==0) )
          refillPlotData(syncPtr, summaryEntry, *itSummary, DISPLAY_N_POINTS, ValidateRefill) ;
        else if((_analyzeCount % REFILL_AFTER_POINTS) == 0)
          refillPlotData(syncPtr, summaryEntry, *itSummary, DISPLAY_N_POINTS, ValidateRefill) ;
        else
          summaryEntry->addcontent(1.0,fabs( (syncPtr->getLiteShotVal()-syncPtr->getValMin()) * syncPtr->getScalingFactor()));
      }

      if (AUTO_DETECTION_ENB) {
        unsigned offByOneStatus = syncPtr->getOffByOneStatus();
        if (offByOneStatus>0) {
          switch (offByOneStatus) {    
            case 1 : 
            case 2 : printf("\n ** SynEr: %s/%d =%u => Both Curves falls in one another by small distance **\n",syncPtr->detInfo.name(syncPtr->detInfo.device()),syncPtr->detInfo.devId(),offByOneStatus); break;
            case 3 : 
            case 4 : printf("\n ** SynEr: %s/%d =%u => Both Curves intersect one another **\n",syncPtr->detInfo.name(syncPtr->detInfo.device()),syncPtr->detInfo.devId(),offByOneStatus); break;
            case 5 : 
            case 6 : printf("\n ** SynEr: %s/%d =%u => Both Curves closely resides at each other sides **\n",syncPtr->detInfo.name(syncPtr->detInfo.device()),syncPtr->detInfo.devId(),offByOneStatus); break;
            case 7 :
            case 8 : printf("\n ** SynEr: %s/%d =%u => Both Curves cover each other entirely with large space  **\n",syncPtr->detInfo.name(syncPtr->detInfo.device()),syncPtr->detInfo.devId(),offByOneStatus); break;
            default: printf("\n ** SynEr: %s/%d =%u => Invalid Status Number **\n",syncPtr->detInfo.name(syncPtr->detInfo.device()),syncPtr->detInfo.devId(),offByOneStatus); break;

          }
        }
      }
    }  

    // For 2-D Plot
    bool includeDarkShot = false;
    if (_detXYPresent) {
      if (_plot2DRefill)
        refill2DPlotData(DISPLAY_N_POINTS,includeDarkShot) ;
      else { 
     	  double dataX = fabs( (_syncPtrDetX->getLiteShotVal()-_syncPtrDetX->getValMin()) * _syncPtrDetX->getScalingFactor());
          double dataY = fabs( (_syncPtrDetY->getLiteShotVal()-_syncPtrDetY->getValMin()) * _syncPtrDetY->getScalingFactor());
        _scatterPlotEntry->xbin(dataX,_scatterPlotBinsCount);
        _scatterPlotEntry->ysum(dataY,_scatterPlotBinsCount);    
        _scatterPlotEntry->nentries(1,_scatterPlotBinsCount);    
        _scatterPlotBinsCount++;
        if(_scatterPlotBinsCount >= DISPLAY_N_POINTS) _scatterPlotBinsCount = 0;    
      }       
    }
  
    if(_notRefilledCount >= DISPLAY_N_POINTS)
      _notRefilledCount = 1; 
    else
      _notRefilledCount++;
    _analyzeCount++;
    if ((_analyzeCount%1000 )== 0)
      printf("AnaEvents = %u noRefiilCnt = %u \n",_analyzeCount, _notRefilledCount); 


    /** Test Cases **/
    _seconds->valid(_clk);
    _minutes->valid(_clk);
    _seconds->addcontent(_clk.seconds()%60); 
    _minutes->addcontent((_clk.seconds()/60)%60);
  }
}


static SummaryAnalysis* _instance = 0;

SummaryAnalysis& SummaryAnalysis::instance() 
{
  if (!_instance)
    _instance = new SummaryAnalysis;
  return *_instance; 
}


void SummaryAnalysis::refillPlotData(SyncAnalysis* syncPtr, EntryTH1F* summaryLiteEntry, EntryTH1F* summaryDarkEntry, unsigned points, unsigned refillType)
{

  findMinMaxRange(syncPtr, points);
  if (syncPtr == _syncPtrDetX) {
    _liteLookUpIndexHighX = _liteLookUpIndexHigh; 
    _darkLookUpIndexHighX = _darkLookUpIndexHigh; 
    _liteLookUpIndexLowX  = _liteLookUpIndexLow;
    _darkLookUpIndexLowX  = _darkLookUpIndexLow;
  }

  if (refillType == ForceRefill)
    fillPlots(syncPtr, summaryLiteEntry, summaryDarkEntry);
  //check change Min & Max in detector databank if +/- margin variation 
  else if ( (fabs(syncPtr->getValMin() - _minVal) > (_margin* _range)) ||
            (fabs(syncPtr->getValMax() - _maxVal) > (_margin* _range))  )
    fillPlots(syncPtr, summaryLiteEntry, summaryDarkEntry);
  else {
    if (_darkShot)
      summaryDarkEntry->addcontent(1.0,fabs( (syncPtr->getDarkShotVal()-syncPtr->getValMin()) * syncPtr->getScalingFactor()));
    else
      summaryLiteEntry->addcontent(1.0,fabs( (syncPtr->getLiteShotVal()-syncPtr->getValMin()) * syncPtr->getScalingFactor()));
  }

  if (AUTO_DETECTION_ENB)
    autoOffByOneDetection(syncPtr);


 } 



void SummaryAnalysis::refill2DPlotData(unsigned points, bool includeDarkShot)
{

  unsigned dataArrayLength  = _syncPtrDetX->liteArrayLength(); 
  double*  dataArrayX       = _syncPtrDetX->getLiteShotArray();
  double*  dataArrayY       = _syncPtrDetY->getLiteShotArray();
  unsigned statShotsFull    = _syncPtrDetX->statLiteShotsFull();
  unsigned lookUpIndexHigh  = _syncPtrDetX->getLiteShotIndex(); 
  unsigned lookUpIndexLow   = 0;
  double minValX = _syncPtrDetX->getValMin();
  double minValY = _syncPtrDetY->getValMin();
  double scaleX  = _syncPtrDetX->getScalingFactor();
  double scaleY  = _syncPtrDetY->getScalingFactor();
  double dataX = 0;
  double dataY = 0;
  unsigned i=0;
  unsigned darkDataIndex = 0;

  //Find Lower and Upper Look-Up Index
  if(lookUpIndexHigh >= points)
    lookUpIndexLow = lookUpIndexHigh - points;
  else if (statShotsFull == 1)
    lookUpIndexLow = dataArrayLength - (points-lookUpIndexHigh);

  if( lookUpIndexHigh < lookUpIndexLow) {
    for (i=lookUpIndexLow; i < dataArrayLength; i++) {
      dataX = fabs((*(dataArrayX+i) - minValX) * scaleX);
      dataY = fabs((*(dataArrayY+i) - minValY) * scaleY);
      _scatterPlotEntry->xbin(dataX,i-lookUpIndexLow);
      _scatterPlotEntry->ysum(dataY,i-lookUpIndexLow); 
      _scatterPlotEntry->nentries(1,i-lookUpIndexLow);       
    }
    for (i=0; i < lookUpIndexHigh; i++){
      dataX = fabs((*(dataArrayX+i) - minValX) * scaleX);
      dataY = fabs((*(dataArrayY+i) - minValY) * scaleY);
      _scatterPlotEntry->xbin(dataX,lookUpIndexLow+i);
      _scatterPlotEntry->ysum(dataY,lookUpIndexLow+i); 
      _scatterPlotEntry->nentries(1,lookUpIndexLow+i);          
    }
    if(includeDarkShot) {
      dataArrayX    = _syncPtrDetX->getDarkShotArray();
      dataArrayY    = _syncPtrDetY->getDarkShotArray();
      darkDataIndex = _syncPtrDetX->getDarkShotIndex(); 
      for (i=0; i < darkDataIndex; i++){
        dataX = fabs((*(dataArrayX+i) - minValX) * scaleX);
        dataY = fabs((*(dataArrayY+i) - minValY) * scaleY);
        _scatterPlotEntry->xbin(dataX,lookUpIndexHigh+i);
        _scatterPlotEntry->ysum(dataY,lookUpIndexHigh+i); 
        _scatterPlotEntry->nentries(1,lookUpIndexHigh+i);          
      }
    }
    _scatterPlotBinsCount = lookUpIndexHigh + darkDataIndex;
  } else {
    for (i=lookUpIndexLow; i <lookUpIndexHigh; i++){
      dataX = fabs((*(dataArrayX+i) - minValX) * scaleX);
      dataY = fabs((*(dataArrayY+i) - minValY) * scaleY);
      _scatterPlotEntry->xbin(dataX,i-lookUpIndexLow);
      _scatterPlotEntry->ysum(dataY,i-lookUpIndexLow); 
      _scatterPlotEntry->nentries(1,i-lookUpIndexLow);          
    }
    if(includeDarkShot) {
      dataArrayX    = _syncPtrDetX->getDarkShotArray();
      dataArrayY    = _syncPtrDetY->getDarkShotArray();
      darkDataIndex = _syncPtrDetX->getDarkShotIndex(); 
      for (i=0; i < darkDataIndex; i++){
        dataX = fabs((*(dataArrayX+i) - minValX) * scaleX);
        dataY = fabs((*(dataArrayY+i) - minValY) * scaleY);
        _scatterPlotEntry->xbin(dataX,(lookUpIndexHigh - lookUpIndexLow)+i);
        _scatterPlotEntry->ysum(dataY,(lookUpIndexHigh - lookUpIndexLow)+i); 
        _scatterPlotEntry->nentries(1,(lookUpIndexHigh - lookUpIndexLow)+i);          
      }
    }
    _scatterPlotBinsCount = (lookUpIndexHigh - lookUpIndexLow) + darkDataIndex; 
  }
} 


void SummaryAnalysis::findMinMaxRange(SyncAnalysis* syncPtr, unsigned points)
{
  unsigned dataArrayLength = syncPtr->liteArrayLength(); 
  double*  dataArray   = syncPtr->getLiteShotArray();
  unsigned statShotsFull = syncPtr->statLiteShotsFull();
  unsigned lookUpIndexHigh = syncPtr->getLiteShotIndex(); 
  unsigned lookUpIndexLow  = 0;
  unsigned i=0;

  for(unsigned j=0; j<2 ; j++) {
    if (j == 1) {
      dataArrayLength = syncPtr->darkArrayLength(); 
      dataArray   = syncPtr->getDarkShotArray();
      statShotsFull = syncPtr->statDarkShotsFull(); 
      lookUpIndexHigh = syncPtr->getDarkShotIndex(); 
      lookUpIndexLow  = 0;     
    }   

    //Find Lower and Upper Look-Up Index
    if(lookUpIndexHigh >= points)
      lookUpIndexLow = lookUpIndexHigh - points;
    else if (statShotsFull == 1)
      lookUpIndexLow = dataArrayLength - (points-lookUpIndexHigh);

    //Find Min & Max Val for current Slot
    _minVal = *(dataArray + lookUpIndexLow);
    _maxVal = *(dataArray + lookUpIndexLow);
  
    if( lookUpIndexHigh < lookUpIndexLow) {
      for (i=lookUpIndexLow; i < dataArrayLength; i++) {
        if (_minVal > *(dataArray+i)) _minVal = *(dataArray+i);
        if (_maxVal < *(dataArray+i)) _maxVal = *(dataArray+i);
      }
      for (i=0; i < lookUpIndexHigh; i++) {
        if (_minVal > *(dataArray+i)) _minVal = *(dataArray+i);
        if (_maxVal < *(dataArray+i)) _maxVal = *(dataArray+i);
      }
    }
    else {
      for (i=lookUpIndexLow; i <lookUpIndexHigh; i++) {
        if (_minVal > *(dataArray+i)) _minVal = *(dataArray+i);
        if (_maxVal < *(dataArray+i)) _maxVal = *(dataArray+i);
      }
    }


    if (j == 1) {
      _darkMinVal          = _minVal;
      _darkMaxVal          = _maxVal; 
      _darkLookUpIndexHigh = lookUpIndexHigh;    
      _darkLookUpIndexLow  = lookUpIndexLow;
    } else {
      _liteMinVal          = _minVal;
      _liteMaxVal          = _maxVal; 
      _liteLookUpIndexHigh = lookUpIndexHigh;    
      _liteLookUpIndexLow  = lookUpIndexLow;
    } 
  }


  // consider darkMin & darkMax only if dark shot is present
  if ( (_darkLookUpIndexHigh > 0)  || (syncPtr->statDarkShotsFull() == 1) )  {
    _maxVal = (_liteMaxVal >= _darkMaxVal) ? _liteMaxVal : _darkMaxVal;
    _minVal = (_liteMinVal <= _darkMinVal) ? _liteMinVal : _darkMinVal;
  } else {
    _maxVal = _liteMaxVal;
    _minVal = _liteMinVal;
  }
 
  // set Min-Max on % Margin
  _range = _maxVal- _minVal;
  _margin = ( (double) MIN_MAX_MARGIN_PERCENT)/100.0;
  _maxVal = _maxVal + (_margin* _range);
  _minVal = _minVal - (_margin* _range);

  if (_maxVal < 0) _maxVal = 0;
  if (_minVal < 0) _minVal = 0;
  _range = _maxVal- _minVal;

}

void SummaryAnalysis::fillPlots(SyncAnalysis* syncPtr, EntryTH1F* summaryLiteEntry, EntryTH1F* summaryDarkEntry)
{
  double scalingFactor = (double)(X_MAX - X_MIN)/(_maxVal-_minVal);
  syncPtr->setValMin(_minVal); 
  syncPtr->setValMax(_maxVal);
  syncPtr->setScalingFactor(scalingFactor);    

  EntryTH1F* summaryEntry = summaryLiteEntry;
  unsigned dataArrayLength = syncPtr->liteArrayLength(); 
  double*  dataArray = syncPtr->getLiteShotArray();
  unsigned lookUpIndexHigh = _liteLookUpIndexHigh;
  unsigned lookUpIndexLow  = _liteLookUpIndexLow;
  unsigned i=0;

  for(unsigned j=0; j<2 ; j++) { 
    if (j == 1) { 
      summaryEntry = summaryDarkEntry;
      dataArrayLength = syncPtr->darkArrayLength(); 
      dataArray = syncPtr->getDarkShotArray();
      lookUpIndexHigh = _darkLookUpIndexHigh;
      lookUpIndexLow  = _darkLookUpIndexLow;    
    }   
    summaryEntry->clear();
    if( lookUpIndexHigh < lookUpIndexLow) {
      for (i=lookUpIndexLow; i < dataArrayLength; i++) 
        summaryEntry->addcontent(1.0,fabs((*(dataArray+i) - _minVal) * scalingFactor) );
      for (i=0; i < lookUpIndexHigh; i++) 
        summaryEntry->addcontent(1.0,fabs((*(dataArray+i) - _minVal) * scalingFactor) );
    } else {
      for (i=lookUpIndexLow; i <lookUpIndexHigh; i++) 
        summaryEntry->addcontent(1.0,fabs((*(dataArray+i) - _minVal) * scalingFactor) );
    } 
  }

  //check for 2D Plot refill
  if (syncPtr->getDataType() == H2D_DET_X_DATATYPE)   _plot2DRefill = true;
  if (syncPtr->getDataType() == H2D_DET_Y_DATATYPE)   _plot2DRefill = true;
 
}

 
void SummaryAnalysis::autoOffByOneDetection(SyncAnalysis* syncPtr)
{
    unsigned offByOneStatus = 0;
    double tolerence = _margin * _range;
    if ( (_darkLookUpIndexHigh > 0)  || (syncPtr->statDarkShotsFull() == 1) )  {
      //lite & dark Curve falls in one another by small distance
      if      (fabs(_liteMaxVal- _darkMaxVal) < tolerence) offByOneStatus = 1;
      else if (fabs(_liteMinVal- _darkMinVal) < tolerence) offByOneStatus = 2;
      //lite & dark Curve intersect one another
      else if ((_liteMinVal<= _darkMaxVal) && (_darkMaxVal<= _liteMaxVal)) offByOneStatus = 3;
      else if ((_liteMinVal<= _darkMinVal) && (_darkMinVal<= _liteMaxVal)) offByOneStatus = 4;
      //lite & dark Curve closely resides at each other sides
      else if ((_darkMinVal<= _liteMaxVal) && ((_liteMaxVal- _darkMinVal)<tolerence) ) offByOneStatus = 5;
      else if ((_darkMaxVal<= _liteMinVal) && ((_liteMinVal- _darkMaxVal)<tolerence) ) offByOneStatus = 6;
      //lite & dark Curve cover each other entirely with large space
      else if ((_liteMinVal<= _darkMinVal) && (_liteMaxVal>= _darkMaxVal)) offByOneStatus = 7;
      else if ((_darkMinVal<= _liteMinVal) && (_darkMaxVal>= _liteMaxVal)) offByOneStatus = 8;
    }

    syncPtr->setOffByOneStatus(offByOneStatus);

}

