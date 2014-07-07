#ifndef PSANA_EVENTLOOP_H
#define PSANA_EVENTLOOP_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: EventLoop.h 3781 2012-06-11 01:01:41Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class EventLoop.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <deque>
#include <vector>
#include <utility>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psana/Module.h"
#include "PSEnv/Env.h"
#include "PSEvt/Event.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------
namespace psana {
class InputIter;
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
 *  @brief Implementation of the event loop for psana.
 *
 *  The purpose of this class is to provide iteration over "events"
 *  in psana framework with well-defined properties. Event in this
 *  context means not only regular events but also transitions
 *  like BeginRun, EndRun, etc. Instance of this class acts like
 *  iterator which returns two items for each iteration -
 *  event type and event contents (PSEvt::Event object). It guarantees
 *  correct nesting of transitions and events, so that regular events
 *  happen only inside BeginCalibCycle/EndCalibCycle, which in turn
 *  happen only inside BeginRun/EndRun. On every iteration this instance
 *  calls corresponding method of the user modules before returning
 *  event to caller so that user modules can add more data to event.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id: EventLoop.h 3781 2012-06-11 01:01:41Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andy Salnikov
 */

class EventLoop : boost::noncopyable {
public:

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
  };

  typedef boost::shared_ptr<PSEvt::Event> EventPtr;
  typedef boost::shared_ptr<PSEnv::Env> EnvPtr;
  typedef std::pair<EventType, EventPtr> value_type;

  /**
   *  @brief Constructor takes instance of input module, and a list of
   *  user modules.
   */
  EventLoop(const boost::shared_ptr<InputModule>& inputModule,
            const std::vector<boost::shared_ptr<Module> >& modules,
            const boost::shared_ptr<PSEnv::Env>& env);

  // Destructor
  ~EventLoop();

  /// Get environment object
  PSEnv::Env& env() const;

  /**
   *  Method that runs one iteration and returns event type,
   *  and event object.
   */
  value_type next();

  /**
   *  @brief "Return" an event back to the input stream. 
   *  
   *  If you want to look ahead for the next event, to check its type, 
   *  for example, then you can call  next() followed by putback(). 
   */
  void putback(const value_type& value) { m_values.push_front(value); }

protected:

private:

  enum {NumEventTypes = None+1};
  
  typedef void (Module::*ModuleMethod)(EventPtr evt, EnvPtr env);

  /**
   *  Calls a method on all modules and returns summary  status.
   *
   *  @param[in] modules  List of modules
   *  @param[in] method   Pointer to the member function
   *  @param[in] evt      Event object
   *  @param[in] env      Environment object
   *  @param[in] ignoreSkip Should be set to false for event() method, true for all others
   */
  Module::Status callModuleMethod(ModuleMethod method, EventPtr evt, EnvPtr env, bool ignoreSkip);


  boost::shared_ptr<InputIter> m_inputIter;
  std::vector<boost::shared_ptr<Module> > m_modules;
  ModuleMethod m_eventMethods[NumEventTypes];
  std::deque<value_type> m_values;
};

} // namespace psana

#endif // PSANA_EVENTLOOP_H
