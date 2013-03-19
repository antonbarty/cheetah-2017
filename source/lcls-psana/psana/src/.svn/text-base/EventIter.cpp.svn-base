//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class EventIter...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/EventIter.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

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
EventIter::EventIter ()
  : m_evtLoop()
  , m_stopType(EventLoop::None)
{
}

EventIter::EventIter (const boost::shared_ptr<EventLoop>& evtLoop, EventLoop::EventType stopType)
  : m_evtLoop(evtLoop)
  , m_stopType(stopType)
{
}

//--------------
// Destructor --
//--------------
EventIter::~EventIter ()
{
}

/// get next event, returns zero pointer when done
boost::shared_ptr<PSEvt::Event>
EventIter::next()
{
  boost::shared_ptr<PSEvt::Event> result;
  if (m_stopType == EventLoop::Event) {
    // means iteration already finished
    return result;
  }

  while (true) {
    EventLoop::value_type nxt = m_evtLoop->next();
    if (nxt.first == EventLoop::None) {
      // no events left
      m_stopType = EventLoop::Event;
      break;
    } else if (nxt.first == m_stopType) {
      // we stop here, return event back to the stream, someone else
      // may be interested in it
      m_evtLoop->putback(nxt);
      m_stopType = EventLoop::Event;
      break;
    } else if (nxt.first == EventLoop::Event) {
      // found event
      result = nxt.second;
      break;
    }
  }
  return result;
}

} // namespace psana
