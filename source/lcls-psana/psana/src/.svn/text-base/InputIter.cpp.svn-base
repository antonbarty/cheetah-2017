//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class InputIter...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/InputIter.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <algorithm>
#include <iostream>
#include <iterator>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psana/Exceptions.h"
#include "psana/InputModule.h"
#include "PSEvt/ProxyDict.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace {

  const char* logger = "InputIter";

}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
InputIter::InputIter (const boost::shared_ptr<InputModule>& inputModule,
    const boost::shared_ptr<PSEnv::Env>& env)
  : m_inputModule(inputModule)
  , m_env(env)
  , m_finished(false)
  , m_state(StateNone)
  , m_values()
{
  m_newStateEventType[StateNone] = None;
  m_newStateEventType[StateConfigured] = BeginJob;
  m_newStateEventType[StateRunning] = BeginRun;
  m_newStateEventType[StateScanning] = BeginCalibCycle;

  m_closeStateEventType[StateNone] = None;
  m_closeStateEventType[StateConfigured] = EndJob;
  m_closeStateEventType[StateRunning] = EndRun;
  m_closeStateEventType[StateScanning] = EndCalibCycle;
}

//--------------
// Destructor --
//--------------
InputIter::~InputIter ()
{
  // call endJob if has not been called yet
  if (m_state != StateNone) {
    EventPtr evt = boost::make_shared<PSEvt::Event>(boost::make_shared<PSEvt::ProxyDict>());
    m_inputModule->endJob(*evt, *m_env);
  }
}

/**
 *  Method that runs one iteration and returns event type,
 *  and event object.
 */
InputIter::value_type
InputIter::next()
{
  value_type result(None, boost::shared_ptr<PSEvt::Event>());
  
  // if queue is not empty return first transition in the queue
  if (not m_values.empty()) {
    result = m_values.front();
    m_values.pop_front();
    return result;
  }

  if (m_finished) return result;

  WithMsgLog(logger, debug, out) {
    out << "enter -- m_state: " << m_state;
    if (not m_values.empty()) {
      out << " m_values:";
      for (std::deque<value_type>::const_iterator it = m_values.begin(); it != m_values.end(); ++it) {
        out <<  " " << it->first;
      }
    }
  }

  if (m_state == StateNone) {
    // run beginJob for input module
    EventPtr evt = boost::make_shared<PSEvt::Event>(boost::make_shared<PSEvt::ProxyDict>());
    m_inputModule->beginJob(*evt, *m_env);
    newState(StateConfigured, evt);
  }

  // if we don't have any saved transitions on stack then get next
  // transition from input module, decide what to do with it
  while (m_values.empty()) {

    // Create event object
    EventPtr evt = boost::make_shared<PSEvt::Event>(boost::make_shared<PSEvt::ProxyDict>());

    // run input module to populate event
    InputModule::Status istat = m_inputModule->event(*evt, *m_env);
    MsgLog(logger, debug, "input.event() returned " << istat);

    // check input status
    if (istat == InputModule::Skip) continue;
    if (istat == InputModule::Stop) break;
    if (istat == InputModule::Abort) {
      MsgLog(logger, info, "Input module requested abort");
      throw ExceptionAbort(ERR_LOC, "Input module requested abort");
    }

    // dispatch event to particular method based on event type
    if (istat == InputModule::DoEvent) {

      // Make sure that Begin called for all states
      this->newState(StateScanning, evt);
      m_values.push_back(value_type(Event, evt));

    } else {

      State unwindTo = StateNone;
      State newState = StateNone;
      if (istat == InputModule::BeginRun) {
        unwindTo = StateConfigured;
        newState = StateRunning;
      } else if (istat == InputModule::BeginCalibCycle) {
        unwindTo = StateRunning;
        newState = StateScanning;
      } else if (istat == InputModule::EndCalibCycle) {
        unwindTo = StateRunning;
      } else if (istat == InputModule::EndRun) {
        unwindTo = StateConfigured;
      }

      unwind(unwindTo, evt);
      if (newState != StateNone) {
        this->newState(newState, evt);
      }

    }
  }

  if (m_values.empty()) {
    // means we reached the end, time to call endJob
    EventPtr evt = boost::make_shared<PSEvt::Event>(boost::make_shared<PSEvt::ProxyDict>());
    m_inputModule->endJob(*evt, *m_env);
    unwind(StateNone, evt);
    m_finished = true;
  }

  // return first transition in the queue
  if (not m_values.empty()) {
    result = m_values.front();
    m_values.pop_front();
  }

  WithMsgLog(logger, debug, out) {
    out << "exit -- m_state: " << m_state;
    if (not m_values.empty()) {
      out << " m_values:";
      for (std::deque<value_type>::const_iterator it = m_values.begin(); it != m_values.end(); ++it) {
        out <<  " " << it->first;
      }
    }
  }

  return result;
}


void
InputIter::finish()
{
  // means we reached the end, time to call endJob
  EventPtr evt = boost::make_shared<PSEvt::Event>(boost::make_shared<PSEvt::ProxyDict>());
  m_inputModule->endJob(*evt, *m_env);
  unwind(StateNone, evt);
  m_finished = true;
}


void
InputIter::newState(State state, const EventPtr& evt)
{
  MsgLog(logger, trace, "newState " << state);

  // make sure that previous state is also in the stack
  if (int(m_state) < int(state-1)) {
    // use different event instance for it
    EventPtr evt = boost::make_shared<PSEvt::Event>(boost::make_shared<PSEvt::ProxyDict>());
    newState(State(state-1), evt);
  }

  if (int(m_state) < int(state)) {
    // save the state
    m_state = state;
  
    // store result
    if (m_newStateEventType[state] != None) {
      m_values.push_back(value_type(m_newStateEventType[state], evt));
    }
  }
}


void
InputIter::closeState(const EventPtr& evt)
{
  MsgLog(logger, trace, "closeState " << m_state);

  // store result
  if (m_closeStateEventType[m_state] != None) {
    m_values.push_back(value_type(m_closeStateEventType[m_state], evt));
  }

  // go back to previous state
  m_state = State(m_state-1);
}


void
InputIter::unwind(State newState, const EventPtr& evt)
{
  while (m_state > newState+1) {
    // use different event instance for it
    EventPtr evt = boost::make_shared<PSEvt::Event>(boost::make_shared<PSEvt::ProxyDict>());
    closeState(evt);
  }
  if (m_state > newState) {
    closeState(evt);
  }
}

// formatting for InputIter::EventType enum
std::ostream&
operator<<(std::ostream& out, InputIter::EventType type)
{
  const char* str = "???";
  switch (type) {
  case InputIter::None:
    str = "None";
    break;
  case InputIter::BeginJob:
    str = "BeginJob";
    break;
  case InputIter::BeginRun:
    str = "BeginRun";
    break;
  case InputIter::BeginCalibCycle:
    str = "BeginCalibCycle";
    break;
  case InputIter::Event:
    str = "Event";
    break;
  case InputIter::EndCalibCycle:
    str = "EndCalibCycle";
    break;
  case InputIter::EndRun:
    str = "EndRun";
    break;
  case InputIter::EndJob:
    str = "EndJob";
    break;
  case InputIter::NumEventTypes:
    break;
  }
  return out << str;  
}

// formatting for InputIter::State enum
std::ostream&
operator<<(std::ostream& out, InputIter::State state)
{
  const char* str = "???";
  switch (state) {
  case InputIter::StateNone:
    str = "StateNone";
    break;
  case InputIter::StateConfigured:
    str = "StateConfigured";
    break;
  case InputIter::StateRunning:
    str = "StateRunning";
    break;
  case InputIter::StateScanning:
    str = "StateScanning";
    break;
  case InputIter::NumStates:
    break;
  }
  return out << str;  
}

} // namespace psana
