//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ImgPeakFilter...
//
// Author List:
//      Mikhail S. Dubrovin
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ImgAlgos/ImgPeakFilter.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
// to work with detector data include corresponding 
// header from psddl_psana package
//#include "psddl_psana/acqiris.ddl.h"
#include "PSEvt/EventId.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace ImgAlgos;
PSANA_MODULE_FACTORY(ImgPeakFilter)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace ImgAlgos {

//----------------
// Constructors --
//----------------
ImgPeakFilter::ImgPeakFilter (const std::string& name)
  : Module(name)
  , m_src()
  , m_key()
  , m_filterIsOn()
  , m_thr_peak()
  , m_thr_total()
  , m_thr_npeaks()
  , m_print_bits()
  , m_count(0)
  , m_selected(0)
{
  // get the values from configuration or use defaults
  m_src        = configStr("source", "DetInfo(:Opal1000)");
  m_key        = configStr("key",       "peaks");
  m_filterIsOn = config   ("filterIsOn",   true);
  m_thr_peak   = config   ("threshold_peak",  0);
  m_thr_total  = config   ("threshold_total", 0);
  m_thr_npeaks = config   ("n_peaks_min",     1);
  m_print_bits = config   ("print_bits",      0);
}

//--------------
// Destructor --
//--------------
ImgPeakFilter::~ImgPeakFilter ()
{
}

/// Method which is called once at the beginning of the job
void 
ImgPeakFilter::beginJob(Event& evt, Env& env)
{
  if( m_print_bits & 1 ) printInputParameters();  
}

/// Method which is called at the beginning of the run
void 
ImgPeakFilter::beginRun(Event& evt, Env& env)
{
}

/// Method which is called at the beginning of the calibration cycle
void 
ImgPeakFilter::beginCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
ImgPeakFilter::event(Event& evt, Env& env)
{
  ++ m_count;

  if ( !m_filterIsOn ) { ++ m_selected; return; } // If the filter is OFF then event is selected

  shared_ptr< vector<Peak> > peaks = evt.get(m_src, m_key, &m_actualSrc);
  if (peaks.get()) {
    m_peaks = peaks.get();
    if ( eventIsSelected(evt) ) { ++ m_selected; return; } // event is selected
  }

  skip(); return; // if event is discarded
}
  
/// Method which is called at the end of the calibration cycle
void 
ImgPeakFilter::endCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called at the end of the run
void 
ImgPeakFilter::endRun(Event& evt, Env& env)
{
}

//--------------------
/// Method which is called once at the end of the job
void 
ImgPeakFilter::endJob(Event& evt, Env& env)
{
  if( m_print_bits & 2 ) MsgLog(name(), info, "Job summary: number of selected events = " << m_selected << " of total " << m_count);
}

//--------------------

bool
ImgPeakFilter::eventIsSelected(Event& evt)
{
  if( m_print_bits & 4 ) printPeaks();  
  if( m_print_bits & 8 ) printEventId(evt);

  return peakSelector();
}

//--------------------
// Loop over vector of peaks and count peaks fulfiled the filter conditions
bool
ImgPeakFilter::peakSelector()
{
  unsigned n_selected_peaks = 0;

  for( vector<Peak>::const_iterator itv  = m_peaks->begin();
                                    itv != m_peaks->end(); itv++ ) {
    if ( itv->ampmax > m_thr_peak 
      && itv->amptot > m_thr_total ) n_selected_peaks++;
  }

  if ( n_selected_peaks >= m_thr_npeaks ) return true;
  else                                    return false;
}

//--------------------

void 
ImgPeakFilter::printPeaks()
{
  MsgLog( name(), info, "Vector of peaks of size " << m_peaks->size() );

  for( vector<Peak>::const_iterator itv  = m_peaks->begin();
                                    itv != m_peaks->end(); itv++ ) {

      cout << "  x="      << itv->x     
           << "  y="      << itv->y     
           << "  ampmax=" << itv->ampmax
           << "  amptot=" << itv->amptot
           << "  npix="   << itv->npix  
           << endl; 
  }
}

//--------------------

// Print input parameters
void 
ImgPeakFilter::printInputParameters()
{
  WithMsgLog(name(), info, log) {
    log << "\n Input parameters:"
        << "\n source     : "     << m_src
        << "\n key        : "     << m_key      
        << "\n filterIsOn : "     << m_filterIsOn   
        << "\n thr_peak   : "     << m_thr_peak
        << "\n thr_total  : "     << m_thr_total
        << "\n thr_npeaks : "     << m_thr_npeaks
        << "\n print_bits : "     << m_print_bits
        << "\n";     
  }
}

//--------------------

void 
ImgPeakFilter::printEventId(Event& evt)
{
  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    MsgLog( name(), info, "event ID: " << *eventId);
  }
}

//--------------------

} // namespace ImgAlgos
