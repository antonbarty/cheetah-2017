//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: RunIter.cpp 5381 2013-02-07 01:50:39Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class RunIter...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/RunIter.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "PSEvt/EventId.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
RunIter::RunIter ()
  : m_evtLoop()
{
}

/// Constructor takes event loop instance.
RunIter::RunIter (const boost::shared_ptr<EventLoop>& evtLoop)
  : m_evtLoop(evtLoop)
{
}

//--------------
// Destructor --
//--------------
RunIter::~RunIter ()
{
}

/// get next scan, when done returns object which is convertible to "false"
RunIter::value_type 
RunIter::next()
{
  RunIter::value_type result;

  // Go to a BeginRun transition
  while (true) {
    EventLoop::value_type nxt = m_evtLoop->next();
    if (nxt.first == EventLoop::None) {
      // nothing left there
      break;
    } else if (nxt.first == EventLoop::BeginRun) {
      // found it, try to get run number from current event
      boost::shared_ptr<PSEvt::EventId> eid = nxt.second->get();
      int run = eid ? eid->run() : -1 ;
      result = RunIter::value_type(m_evtLoop, run);
      break;
    }
  }

  return result;
}

} // namespace psana
