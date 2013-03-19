#ifndef PSANA_EVENTITER_H
#define PSANA_EVENTITER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class EventIter.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <boost/shared_ptr.hpp>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psana/EventLoop.h"
#include "PSEvt/Event.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana {

/// @addtogroup psana

/**
 *  @ingroup psana
 *
 *  @brief Class representing iterator over events.
 *
 *  This class uses EventLoop class as a source of raw events, it then
 *  selects events of "Event" type and returns them one by one when
 *  next() method is called. Iteration stops when there are no more
 *  events left in the input source or when specified "stop" event
 *  type is reached. To iterate over all events from begin to end use
 *  None event type, to stop at the end of the run use EndRun type,
 *  to stop at the end of the scan use EndCalibSycle event type.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class EventIter {
public:

  /// Default constructor makes invalid iterator
  EventIter () ;

  /**
   *  @brief Constructor takes event loop instance and "stop event type".
   *
   *  Do not use EventLoop::Event for stop type, first it does not make
   *  sense, second this iterator uses it for special purpose.
   */
  EventIter (const boost::shared_ptr<EventLoop>& evtLoop, EventLoop::EventType stopType);

  // Destructor
  ~EventIter();

  /// get next event, returns zero pointer when done
  boost::shared_ptr<PSEvt::Event> next();

protected:

private:

  // Data members
  boost::shared_ptr<EventLoop> m_evtLoop;
  EventLoop::EventType m_stopType;

};

} // namespace psana

#endif // PSANA_EVENTITER_H
