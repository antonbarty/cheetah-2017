//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class TimeStampFilter...
//
// Author List:
//      Mikhail S. Dubrovin
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ImgAlgos/TimeStampFilter.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
// to work with detector data include corresponding 
// header from psddl_psana package
#include "PSEvt/EventId.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace ImgAlgos;
PSANA_MODULE_FACTORY(TimeStampFilter)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace ImgAlgos {

//----------------
// Constructors --
//----------------
TimeStampFilter::TimeStampFilter (const std::string& name)
  : Module(name)
  , m_str_tstamp_min()
  , m_str_tstamp_max()
  , m_filterIsOn()
  , m_print_bits()
  , m_count(0)
  , m_selected(0)
{
  // get the values from configuration or use defaults

  m_def_tstamp_min = "1970-01-01 00:00:00.000000000";
  m_def_tstamp_max = "2100-01-01 00:00:00.000000000";
  m_def_tsinterval =  m_def_tstamp_min + "/" + m_def_tstamp_max;

  m_str_tstamp_min = configStr("tstamp_min", m_def_tstamp_min);
  m_str_tstamp_max = configStr("tstamp_max", m_def_tstamp_max);
  m_str_tsinterval = configStr("tsinterval", m_def_tsinterval);

  m_filterIsOn = config("filterIsOn", true);
  m_print_bits = config("print_bits", 0);

  setInputParameters();
}

//--------------
// Destructor --
//--------------
TimeStampFilter::~TimeStampFilter ()
{
}

/// Method which is called once at the beginning of the job
void 
TimeStampFilter::beginJob(Event& evt, Env& env)
{
  if( m_filterIsOn && m_print_bits & 1 ) printInputParameters();
}

/// Method which is called at the beginning of the run
void 
TimeStampFilter::beginRun(Event& evt, Env& env)
{
}

/// Method which is called at the beginning of the calibration cycle
void 
TimeStampFilter::beginCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
TimeStampFilter::event(Event& evt, Env& env)
{
  ++ m_count;

  if ( !m_filterIsOn )  { ++ m_selected; return; } // If the filter is OFF then event is selected

  getTimeStamp(evt);
  if( m_print_bits & 1<<2 ) printEventId(evt);

  if ( ! isSelected() ) { skip(); return; } // event is discarded

  if( m_print_bits & 1<<3 ) printTimeStamp();

  ++ m_selected; return; // event is selected
}
  
/// Method which is called at the end of the calibration cycle
void 
TimeStampFilter::endCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called at the end of the run
void 
TimeStampFilter::endRun(Event& evt, Env& env)
{
}

/// Method which is called once at the end of the job
void 
TimeStampFilter::endJob(Event& evt, Env& env)
{
  if( m_filterIsOn && m_print_bits & 1<<1 ) MsgLog(name(), info, "Number of selected events = " << m_selected << " of total " << m_count);
}


//--------------------
/// Set input parameters
void 
TimeStampFilter::setInputParameters()
{
  if( m_str_tsinterval != m_def_tsinterval ) {
      size_t pos = m_str_tsinterval.find_first_of('/');
      if (pos!=string::npos){
	m_str_tstamp_min = m_str_tsinterval.substr(0,pos);
	m_str_tstamp_max = m_str_tsinterval.substr(pos+1);

	// Remove trailing spaces in the beginning and the end of the timestamp
	size_t pos1 = m_str_tstamp_min.find_last_not_of(' '); 
	m_str_tstamp_min = m_str_tstamp_min.substr(0,pos1+1);

	size_t pos2 = m_str_tstamp_max.find_first_not_of(' '); 
	m_str_tstamp_max = m_str_tstamp_max.substr(pos2);

        //cout << "\n m_str_tstamp_min: " <<  m_str_tstamp_min << endl;  
        //cout << "\n m_str_tstamp_max: " <<  m_str_tstamp_max << endl;
      }
    }

  m_tmin = new PSTime::Time(m_str_tstamp_min);
  m_tmax = new PSTime::Time(m_str_tstamp_max);
}

//--------------------
/// Print input parameters
void 
TimeStampFilter::printInputParameters()
{
  WithMsgLog(name(), info, log) {
    log << "\n Input parameters:"
        << "\n m_str_tstamp_min: " <<  m_str_tstamp_min   
        << "\n m_str_tstamp_max: " <<  m_str_tstamp_max
        << "\n m_str_tsinterval: " <<  m_str_tsinterval
        << "\n m_tmin: "           << *m_tmin << " or " << m_tmin->sec() << " sec, "  << " " << m_tmin->nsec() << " nsec" 
        << "\n m_tmax: "           << *m_tmax << " or " << m_tmax->sec() << " sec, "  << " " << m_tmax->nsec() << " nsec" 
        << "\n m_print_bits    : " <<  m_print_bits
        << "\n";
  }
}

//--------------------

void 
TimeStampFilter::printEventId(Event& evt)
{
  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    MsgLog( name(), info, "Event="  << m_count << " ID: " << *eventId);
  }
}

//--------------------

void 
TimeStampFilter::getTimeStamp(Event& evt)
{
  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {

    m_time = eventId->time();
    m_run  = eventId->run(); 

    //std::stringstream ss;
    //ss << hex << t_msec;
    //string hex_msec = ss.str();

    // printTimeStamp();

  }
}

//--------------------

void 
TimeStampFilter::printTimeStamp()
{
  MsgLog( name(), info, " Run="          <<  m_run 
	             << " Event="       <<  m_count 
                     << " Time="        <<  m_time  );
}

//--------------------

bool 
TimeStampFilter::isSelected()
{
  if ( m_time <  *m_tmin  ) return false;
  if ( m_time >= *m_tmax  ) return false;

  return true;
}

//--------------------
//--------------------
} // namespace ImgAlgos
