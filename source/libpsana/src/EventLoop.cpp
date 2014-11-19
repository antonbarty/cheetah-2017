//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: EventLoop.cpp 4327 2012-07-30 17:16:02Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class EventLoop...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/EventLoop.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psana/Exceptions.h"
#include "psana/InputIter.h"
#include "psana/InputModule.h"
#include "PSEvt/ProxyDict.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace {

  const char* logger = "EventLoop";

  // translate InputIter event type into EventLoop event type
  EventLoop::EventType eventType(InputIter::EventType type)
  {
    // they are the same for now
    return EventLoop::EventType(type);
  }
  
}


//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
EventLoop::EventLoop (const boost::shared_ptr<InputModule>& inputModule,
    const std::vector<boost::shared_ptr<Module> >& modules,
    const boost::shared_ptr<PSEnv::Env>& env)
  : m_inputIter(boost::make_shared<InputIter>(inputModule, env))
  , m_modules(modules)
  , m_values()
{
  m_eventMethods[BeginJob] = &Module::beginJob;
  m_eventMethods[EndJob] = &Module::endJob;
  m_eventMethods[BeginRun] = &Module::beginRun;
  m_eventMethods[EndRun] = &Module::endRun;
  m_eventMethods[BeginCalibCycle] = &Module::beginCalibCycle;
  m_eventMethods[EndCalibCycle] = &Module::endCalibCycle;
  m_eventMethods[Event] = &Module::event;
  m_eventMethods[None] = 0;
}

//--------------
// Destructor --
//--------------
EventLoop::~EventLoop ()
{
}

// Get environment object
PSEnv::Env& 
EventLoop::env() const 
{ 
  return m_inputIter->env(); 
}

/**
 *  Method that runs one iteration and returns event type,
 *  and event object.
 */
EventLoop::value_type
EventLoop::next()
{
  value_type result(None, boost::shared_ptr<PSEvt::Event>());

  // return first transition in the queue if the queue is not empty
  if (not m_values.empty()) {
    result = m_values.front();
    m_values.pop_front();
    return result;
  }
  
  while (true) {

    // Get next event from input iterator
    InputIter::value_type evt = m_inputIter->next();
    EventType evtType = ::eventType(evt.first);
    if (evtType == None) break;

    // call corresponding method for all modules
    Module::Status stat = callModuleMethod(m_eventMethods[evtType], evt.second, m_inputIter->envptr(), evtType != Event);
    if (stat == Module::Abort) {
      // stop right here
      throw ExceptionAbort(ERR_LOC, "User module requested abort");
    } else if (stat == Module::Stop) {
      // user module requested stop, signal iterator it's time to finish and continue
      m_inputIter->finish();
    } else {
      // good result, return
      result = value_type(evtType, evt.second);
      break;
    }

  }

  return result;
  
}

//
// Call given method for all defined modules, ignoreSkip should be set
// to false for event() method, true for everything else
//
Module::Status
EventLoop::callModuleMethod(ModuleMethod method, boost::shared_ptr<PSEvt::Event> evt, boost::shared_ptr<PSEnv::Env> env, bool ignoreSkip)
{
  Module::Status stat = Module::OK;

  if (ignoreSkip) {

    // call all modules, do not skip any one of them

    for (std::vector<boost::shared_ptr<Module> >::const_iterator it = m_modules.begin() ; it != m_modules.end() ; ++it) {
      boost::shared_ptr<Module> mod = *it;

      // clear module status
      mod->reset();

      // call the method
      ((*mod).*method)(evt, env);

      // check what module wants to tell us
      if (mod->status() == Module::Skip) {
        // silently ignore Skip
      } else if (mod->status() == Module::Stop) {
        // set the flag but continue
        MsgLog(logger, info, "module " << mod->name() << " requested stop");
        stat = Module::Stop;
      } else if (mod->status() == Module::Abort) {
        // abort immediately
        MsgLog(logger, info, "module " << mod->name() << " requested abort");
        stat = Module::Abort;
        break;
      }
    }

  } else {

    // call all modules, respect Skip flag

    for (std::vector<boost::shared_ptr<Module> >::const_iterator it = m_modules.begin() ; it != m_modules.end() ; ++it) {
      boost::shared_ptr<Module> mod = *it;

      // clear module status
      mod->reset();

      // call the method, skip regular modules if skip status is set, but
      // still call special modules which are interested in all events
      if (stat == Module::OK or mod->observeAllEvents()) {
        ((*mod).*method)(evt, env);
      }

      // check what module wants to tell us
      if (mod->status() == Module::Skip) {

        // Set the skip flag but continue as there may be modules interested in every event
        MsgLog(logger, trace, "module " << mod->name() << " requested skip");
        if (stat == Module::OK) stat = Module::Skip;

        // add special flag to event
        if (not evt->exists<int>("__psana_skip_event__")) {
          evt->put(boost::make_shared<int>(1), "__psana_skip_event__");
        }

      } else if (mod->status() == Module::Stop) {
        // stop right here
        MsgLog(logger, info, "module " << mod->name() << " requested stop");
        stat = Module::Stop;
        break;
      } else if (mod->status() == Module::Abort) {
        // abort immediately
        MsgLog(logger, info, "module " << mod->name() << " requested abort");
        stat = Module::Abort;
        break;
      }
    }

  }

  return stat;
}

Index& EventLoop::index()
{
  return  m_inputModule->index();
}

} // namespace psana
