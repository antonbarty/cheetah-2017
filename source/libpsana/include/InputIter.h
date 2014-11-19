#ifndef PSANA_INPUTITER_H
#define PSANA_INPUTITER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: InputIter.h 7529 2014-01-13 21:45:29Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class InputIter.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <deque>
#include <utility>
#include <iosfwd>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "PSEvt/AliasMap.h"
#include "PSEnv/Env.h"
#include "PSEvt/Event.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------
namespace psana {
class InputModule;
}

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana {

/// @addtogroup psana

/**
 *  @ingroup psana
 *
 *  @brief Implementation of the iterator for input events.
 *
 *  The purpose of this class is to provide iteration over "events"
 *  in psana framework with well-defined properties. Event in this
 *  context means not only regular events but also transitions
 *  like BeginRun, EndRun, etc. Instance of this class acts like
 *  iterator which returns two items for each iteration -
 *  event type and event contents (PSEvt::Event object). It guarantees
 *  correct nesting of transitions and events, so that regular events
 *  happen only inside BeginCalibCycle/EndCalibCycle, which in turn
 *  happen only inside BeginRun/EndRun.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id: InputIter.h 7529 2014-01-13 21:45:29Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andy Salnikov
 */

class InputIter : boost::noncopyable {
public:

  /// State order must not change, state machine depends on ordering
  enum State {StateNone=0, StateConfigured=1, StateRunning=2, StateScanning=3, NumStates=4};

  /// Enumeration for the event types returned by iterator
  enum EventType {
    BeginJob,         ///< Returned at the begin of job
    BeginRun,         ///< Returned at the begin of run
    BeginCalibCycle,  ///< Returned at the begin of calib cycle
    Event,            ///< Returned for regular event
    EndCalibCycle,    ///< Returned at the end of calib cycle
    EndRun,           ///< Returned at the end of run
    EndJob,           ///< Returned at the end of job
    None,             ///< Returned if there are no more events
    NumEventTypes,    ///< Total number off event types
  };

  typedef boost::shared_ptr<PSEvt::Event> EventPtr;
  typedef std::pair<EventType, EventPtr> value_type;

  /**
   *  @brief Constructor takes instance of input module and environment object.
   */
  InputIter(const boost::shared_ptr<InputModule>& inputModule,
            const boost::shared_ptr<PSEnv::Env>& env);

  // DEstructor
  ~InputIter();
  
  /// Get environment object
  PSEnv::Env& env() const { return *m_env; }

  /// Get environment object
  boost::shared_ptr<PSEnv::Env> envptr() const { return m_env; }

  /**
   *  Method that runs one iteration and returns event type,
   *  and event object.
   */
  value_type next();

  /**
   *  Prepare to stop iteration, instructs iterator to stop reading
   *  data from input module and produce standard EndCalibCycle/EndRun/EndJob
   *  sequence on the following calls to next().
   */
  void finish();

protected:

private:

  void newState(State state, const EventPtr& evt);
  void closeState(const EventPtr& evt);
  void unwind(State newState, const EventPtr& evt);

  boost::shared_ptr<InputModule> m_inputModule;
  boost::shared_ptr<PSEnv::Env> m_env;
  bool m_finished;
  State m_state;
  EventType m_newStateEventType[NumStates];
  EventType m_closeStateEventType[NumStates];
  std::deque<value_type> m_values;
  boost::shared_ptr<PSEvt::AliasMap> m_aliasMap;
};

/// formatting for InputIter::EventType enum
std::ostream&
operator<<(std::ostream& out, InputIter::EventType type);

/// formatting for InputIter::State enum
std::ostream&
operator<<(std::ostream& out, InputIter::State state);

} // namespace psana

#endif // PSANA_INPUTITER_H
