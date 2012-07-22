//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ImgPixAmpFilter...
//
// Author List:
//      Mikhail S. Dubrovin
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ImgAlgos/ImgPixAmpFilter.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/acqiris.ddl.h"
#include "PSEvt/EventId.h"
#include "PSTime/Time.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace std;
using namespace ImgAlgos;
PSANA_MODULE_FACTORY(ImgPixAmpFilter)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace ImgAlgos {

//----------------
// Constructors --
//----------------
ImgPixAmpFilter::ImgPixAmpFilter (const std::string& name)
  : Module(name)
  , m_src()
  , m_key()
  , m_threshold()
  , m_xmin()
  , m_xmax()
  , m_ymin()
  , m_ymax()
  , m_numpixmin()
  , m_filter()
  , m_print_bits()
  , m_count(0)
  , m_selected(0)
{
  // get the values from configuration or use defaults
  m_src        = configStr("source", "DetInfo(:Cspad)");
  m_key        = configStr("key",   "Image2D");
  m_threshold  = config   ("threshold",    10);
  m_xmin       = config   ("xmin",          0);
  m_xmax       = config   ("xmax",     100000);
  m_ymin       = config   ("ymin",          0);
  m_ymax       = config   ("ymax",     100000);
  m_numpixmin  = config   ("numPixMin",   100);
  m_filter     = config   ("filterIsOn", true);
  m_print_bits= config    ("print_bits",    0);
}

//--------------
// Destructor --
//--------------
ImgPixAmpFilter::~ImgPixAmpFilter ()
{
}

/// Method which is called once at the beginning of the job
void 
ImgPixAmpFilter::beginJob(Event& evt, Env& env)
{
  m_time = new TimeInterval();
}

/// Method which is called at the beginning of the run
void 
ImgPixAmpFilter::beginRun(Event& evt, Env& env)
{
  if( m_print_bits & 1 ) printInputParameters();
}

/// Method which is called at the beginning of the calibration cycle
void 
ImgPixAmpFilter::beginCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
ImgPixAmpFilter::event(Event& evt, Env& env)
{
  m_time -> startTimeOnce();

  ++ m_count;

  if ( !m_filter )            { ++ m_selected; return; } // If the filter is OFF then event is selected

  if ( getAndProcImage(evt) ) { ++ m_selected; return; } // if event is selected
  else                        { skip();        return; } // if event is discarded
}
  
/// Method which is called at the end of the calibration cycle
void 
ImgPixAmpFilter::endCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called at the end of the run
void 
ImgPixAmpFilter::endRun(Event& evt, Env& env)
{
}

/// Method which is called once at the end of the job
void 
ImgPixAmpFilter::endJob(Event& evt, Env& env)
{
  m_time -> stopTime(m_count);
  if( m_print_bits & 1<<1 ) MsgLog(name(), info, "Number of selected events = " << m_selected << " of total " << m_count);
}

//--------------------
//--------------------
//--------------------
//--------------------

/// Print input parameters
void 
ImgPixAmpFilter::printInputParameters()
{
  WithMsgLog(name(), info, log) {
    log << "\nInput parameters:"
        << "\nsource      : "     << m_src
	<< "\nkey         : "     << m_key      
	<< "\nthreshold   : "     << m_threshold
	<< "\nxmin        : "     << m_xmin     
	<< "\nxmax        : "     << m_xmax     
	<< "\nymin        : "     << m_ymin     
	<< "\nymax        : "     << m_ymax     
	<< "\nnumPixMin   : "     << m_numpixmin
        << "\nm_print_bits: "     << m_print_bits
 	<< "\nfilterIsOn  : "     << m_filter;   
  }
}

//--------------------

bool
ImgPixAmpFilter::getAndProcImage(Event& evt)
{
  shared_ptr< CSPadPixCoords::Image2D<double> > img2d = evt.get(m_src, m_key, &m_actualSrc); 
  if (img2d.get()) {
    MsgLog(name(), debug, "::procImage(...): Get image as Image2D<double>");
    m_img2d = img2d.get();
    return procImage(evt);
  }

  shared_ptr< ndarray<double,2> > img = evt.get(m_src, m_key, &m_actualSrc);
  if (img.get()) {
    MsgLog(name(), debug, "::procImage(...): Get image as ndarray<double,2> and wrap it in Image2D<double>");
    m_img2d = new CSPadPixCoords::Image2D<double>(img->data(),img->shape()[0],img->shape()[1]);
    return procImage(evt);
  }

    return false; // if the image object is not found in evt
}

//--------------------

/// Use input parameters and image dimensions and set the window range
void 
ImgPixAmpFilter::setWindowRange()
{
    static unsigned entrance_counter = 0;
    if( entrance_counter > 0 ) return;
        entrance_counter ++;
  
    m_nrows  = m_img2d -> getNRows();
    m_ncols  = m_img2d -> getNCols();

    m_rowmin = (size_t) min(m_ymin, m_ymax); 
    m_rowmax = (size_t) max(m_ymin, m_ymax);
    m_colmin = (size_t) min(m_xmin, m_xmax);
    m_colmax = (size_t) max(m_xmin, m_xmax);    

    if (m_rowmin < 0        ) m_rowmin = 0;
    if (m_rowmax < 1        ) m_rowmax = 1;
    if (m_rowmin >= m_nrows ) m_rowmin = m_nrows-1;
    if (m_rowmax >= m_nrows ) m_rowmax = m_nrows;

    if (m_colmin < 0        ) m_colmin = 0;
    if (m_colmax < 1        ) m_colmax = 1;
    if (m_colmin >= m_ncols ) m_colmin = m_ncols-1;
    if (m_colmax >= m_ncols ) m_colmax = m_ncols;
}

//--------------------

bool
ImgPixAmpFilter::procImage(Event& evt)
{
    setWindowRange();

    unsigned npix_above_thr = 0;
    double d_threshold = m_threshold;

        for (size_t row = m_rowmin; row < m_rowmax; row++) {
          for (size_t col = m_colmin; col < m_colmax; col++) {

	    if (m_img2d -> getValue(row, col) > d_threshold) npix_above_thr ++;

	  }
	}

	if(  m_print_bits & 4 ||
	    (m_print_bits & 8 && m_count%100 == 0) ) 
                               MsgLog(name(), info, "Event = " << m_count 
                               << "  Npix (Amp>Threshold) : Nmin = " << npix_above_thr << " : " << m_numpixmin);

	if (npix_above_thr > m_numpixmin) {
          if( m_print_bits & 16 ) printEventId(evt);
          return true;
	}

  return false;
}

//--------------------

void 
ImgPixAmpFilter::printEventId(Event& evt)
{
  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    //MsgLog( name(), info, "event ID: " << *eventId);

    //std::string t_str = asStringFormat( "%Y-%m-%d %H:%M:%S%f%z");
    std::string t_str = (eventId->time()).asStringFormat( "%H%M%S");
    time_t t_nsec = (eventId->time()).nsec(); 
    int    t_msec = (int)(0.001 * t_nsec); 

    std::stringstream ss;
    ss << hex << t_msec;
    string hex_msec = ss.str();

    MsgLog( name(), info, "Run="   << eventId->run() 
                     << " Time="   << eventId->time() 
                     << " or "     << t_str.data() 
                     << " nsec="   << t_nsec 
                     << " hex_ms=" << hex_msec);
  }
}


//--------------------

} // namespace ImgAlgos
