#ifndef SummaryAnalysis_hh
#define SummaryAnalysis_hh

#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "ami/app/SyncAnalysis.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryScan.hh"
#include "pdsdata/evr/DataV3.hh" 
#include <list>

namespace Pds {
  class ClockTime;
  class Src;
  class TypeId;
};

namespace Ami {
  class Cds;
  class SyncAnalysis;
  class EntryTH1F;

  class SummaryAnalysis {
  public:
    enum {ForceRefill, ValidateRefill};
    static SummaryAnalysis& instance();
 
  private:
    SummaryAnalysis();
    ~SummaryAnalysis();
  public:  // Handler functions
    void reset    ();
    void clock    (const Pds::ClockTime& clk);
    void configure(const Pds::Src& src, const Pds::TypeId& type, void* payload);
    void event    (const Pds::Src& src, const Pds::TypeId& type, void* payload);
  public:                     // Analysis functions
    void clear    ();         // remove Entry's
    void create   (Cds& cds); // create Entry's
    void analyze  ();         // fill   Entry's
    void insert(SyncAnalysis* h)   { _syncAnalysisPList.push_back(h); }
    void remove(SyncAnalysis* h)   { _syncAnalysisPList.remove(h);    }
    void insertEntry(EntryTH1F* h) { _summaryEntryEList.push_back(h); }
    void removeEntry(EntryTH1F* h) { _summaryEntryEList.remove(h);    }
    void insertEntry2(EntryScan* h) { _summaryEntryE2List.push_back(h);}
    void processAcqirisData(SyncAnalysis* syncPtr);
    void processOpalData(SyncAnalysis* syncPtr);
    void processPrincetonData(SyncAnalysis* syncPtr);
    void processGasDetectorData(SyncAnalysis* syncPtr);
    void processEBeamData(SyncAnalysis* syncPtr);
    void processPhaseCavityData(SyncAnalysis* syncPtr);
    void processPnccdData(SyncAnalysis* syncPtr);
    void processIpimbData(SyncAnalysis* syncPtr);
    void processFccdData(SyncAnalysis* syncPtr);
    void processPulnixData(SyncAnalysis* syncPtr);
    void autoOffByOneDetection(SyncAnalysis* syncPtr);
    void findMinMaxRange(SyncAnalysis* syncPtr, unsigned points);
    void fillPlots(SyncAnalysis* syncPtr, EntryTH1F* summaryLiteEntry, EntryTH1F* summaryDarkEntry);
    void refillPlotData (SyncAnalysis* syncPtr, EntryTH1F* summaryGoodEntry, EntryTH1F* summaryDarkEntry, unsigned points, unsigned refillType) ;
    void refill2DPlotData(unsigned points,bool includeDarkShot);

   private:
    typedef std::list<Ami::SyncAnalysis*> PList;
    typedef std::list<Ami::EntryTH1F*> EList;
    typedef std::list<Ami::EntryScan*> E2List;
    PList  _syncAnalysisPList;
    EList  _summaryEntryEList;
    E2List _summaryEntryE2List;
    Pds::EvrData::DataV3*  _evrEventData;
    bool       _darkShot;
    double     _minVal;
    double     _maxVal;
    double     _range;
    double     _margin;
    double     _liteMinVal;
    double     _liteMaxVal;
    double     _darkMinVal;
    double     _darkMaxVal;
    unsigned   _liteLookUpIndexHigh; 
    unsigned   _darkLookUpIndexHigh; 
    unsigned   _liteLookUpIndexLow;
    unsigned   _darkLookUpIndexLow;
    unsigned   _summaryEntries;
    unsigned   _analyzeCount; 
    unsigned   _liteLookUpIndexHighX; 
    unsigned   _darkLookUpIndexHighX; 
    unsigned   _liteLookUpIndexLowX;
    unsigned   _darkLookUpIndexLowX;
    unsigned   _notRefilledCount;
    bool       _detXYPresent;
    bool       _plot2DRefill;
    SyncAnalysis* _syncPtrDetX;
    SyncAnalysis* _syncPtrDetY;
    EntryScan*    _scatterPlotEntry;
    unsigned      _scatterPlotBinsCount;
 
  };
};

#endif
