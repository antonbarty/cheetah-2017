//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class EventNumberFilter...
//
// Author List:
//      Mikhail S. Dubrovin
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ImgAlgos/EventNumberFilter.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "PSEvt/EventId.h"

//#include "psddl_psana/acqiris.ddl.h"
//#include "PSTime/Time.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace std;
using namespace ImgAlgos;
PSANA_MODULE_FACTORY(EventNumberFilter)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace ImgAlgos {

//----------------
// Constructors --
//----------------
EventNumberFilter::EventNumberFilter (const std::string& name)
  : Module(name)
  , m_filter()
  , m_first()
  , m_last()
  , m_evtstring()
  , m_print_bits()
  , m_count(0)
  , m_selected(0)
{
  // get the values from configuration or use defaults
  m_filter     = config   ("filterIsOn",  true);
  m_first      = config   ("first",          0);
  m_last       = config   ("last",       1<<31);
  m_evtstring  = configStr("evtstring",     ""); 
  m_print_bits= config    ("print_bits",     0);

  parseEvtString();
}

//--------------
// Destructor --
//--------------
EventNumberFilter::~EventNumberFilter ()
{
}

/// Method which is called once at the beginning of the job
void 
EventNumberFilter::beginJob(Event& evt, Env& env)
{
  if( m_filter && (m_print_bits & 1) ) printInputParameters();
}

/// Method which is called at the beginning of the run
void 
EventNumberFilter::beginRun(Event& evt, Env& env)
{
}

/// Method which is called at the beginning of the calibration cycle
void 
EventNumberFilter::beginCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
EventNumberFilter::event(Event& evt, Env& env)
{
  if ( !m_filter )            { ++ m_selected; return; } // If the filter is OFF then event is selected

  ++ m_count;

  if ( m_event_vector_is_empty ) {

    if ( m_count < m_first ) { skip(); return; } // event is discarded
    if ( m_count > m_last  ) { skip(); return; } // event is discarded

  } else {

    if ( ! eventIsInList() ) { skip(); return; } // event is discarded
  }

  shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    if( m_print_bits & 1<<2 ) MsgLog(name(), info, "Select event " << m_count << ", ID: " << *eventId); // "Run: " << eventId->run()
  }

  ++ m_selected; return; // event is selected
}
  
/// Method which is called at the end of the calibration cycle
void 
EventNumberFilter::endCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called at the end of the run
void 
EventNumberFilter::endRun(Event& evt, Env& env)
{
}

/// Method which is called once at the end of the job
void 
EventNumberFilter::endJob(Event& evt, Env& env)
{
  if( !m_filter ) return;
  if( m_print_bits & 1<<1 ) MsgLog(name(), info, "Number of selected events = " << m_selected << " of total " << m_count);
}

//--------------------
//--------------------
//--------------------
//--------------------

/// Print input parameters
void 
EventNumberFilter::printInputParameters()
{
  WithMsgLog(name(), info, log) {
    log << "\n Input parameters:"
	<< "\n m_filter      : "     << m_filter   
	<< "\n m_first       : "     << m_first   
	<< "\n m_last        : "     << m_last  
	<< "\n m_evtstring   : "     << m_evtstring   
	<< "\n m_print_bits  : "     << m_print_bits
	<< "\n m_event_vector_is_empty : "  << m_event_vector_is_empty
        << "\n";

    log << " Total number of events in the list = " << m_events.size() << "   event numbers:\n";

    int count=0;
    std::vector<unsigned>::const_iterator it;
    for(it=m_events.begin(); it!=m_events.end(); it++) {
      log << "  " << *it ;
      if (++count >= 10) {count=0; log << "\n";}
    }
      log << "\n";
  }
}

//--------------------

void 
EventNumberFilter::parseEvtString()
{
  char* separ = ",";
  char rec[80];
  size_t pos;
  size_t pos1=0;
  size_t pos2=0;

  MsgLog(name(), debug, ":parseEvtString(): "  << m_evtstring );

  if (m_evtstring.size() == 0) {m_event_vector_is_empty = true;  return;}
                                m_event_vector_is_empty = false;
  do {
    pos = m_evtstring.find(separ,pos1);
    pos2 = (pos != string::npos) ? pos : m_evtstring.size();
    size_t len = m_evtstring.copy(rec,pos2-pos1,pos1);  rec[len]='\0';
    MsgLog(name(), debug, "Separator or string end is found in pos=" << pos2 << " buf=" << rec);    
    pos1 = pos2 + 1;

    parseOneRecord(rec);

  } while (pos != string::npos);
}

//--------------------

void 
EventNumberFilter::parseOneRecord(char* rec)
{
  MsgLog(name(), debug, ":parseOneRecord: "  << rec );

  char* separ = "-";
  string recstring = rec;
  size_t pos = recstring.find(separ);
  if (pos != string::npos) {

    char field1[20], field2[20];
    size_t len1 = recstring.copy(field1,pos,0);                       field1[len1]='\0';
    size_t len2 = recstring.copy(field2,recstring.size()-pos,pos+1);  field2[len2]='\0';

    unsigned val1 = atoi(field1);
    unsigned val2 = atoi(field2);
    //cout << " separator is found in pos=" << pos << "  values:" << val1 << " and " << val2 << endl;

    for(unsigned i=val1; i<=val2; i++) {
      //cout << " add event to the list =" << i << endl;
       m_events.push_back(i);
    }

  } else {

    unsigned val = atoi(rec);
    //cout << " separator is not found" << "  val=" << val << endl;
    //cout << " add event to the list =" << val << endl;
    m_events.push_back(val);
  }
}

//--------------------
bool
EventNumberFilter::eventIsInList()
{
    std::vector<unsigned>::const_iterator it;
    for(it=m_events.begin(); it!=m_events.end(); it++) {
      if (m_count == *it) return true;
    }
    return false;
}

//--------------------
} // namespace ImgAlgos
