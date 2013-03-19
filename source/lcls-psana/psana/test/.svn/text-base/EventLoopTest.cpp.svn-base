//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Test suite case for the EventLoopTest.
//
//------------------------------------------------------------------------

//---------------
// C++ Headers --
//---------------
#include <boost/make_shared.hpp>
#include <algorithm>
#include <iterator>
#include <iostream>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psana/EventLoop.h"
#include "psana/InputModule.h"
#include "PSEnv/Env.h"

using namespace psana ;

#define BOOST_TEST_MODULE EventLoopTest
#include <boost/test/included/unit_test.hpp>

/**
 * Simple test suite for module EventLoopTest.
 * See http://www.boost.org/doc/libs/1_36_0/libs/test/doc/html/index.html
 */

namespace {

// Implementation of the InputModulle which generates predefined sequences of events
class TestInputModule: public InputModule {
public:
  
  TestInputModule(const InputModule::Status states[], int nstates) : InputModule("TestInputModule")
  {
    std::copy(states, states+nstates, std::back_inserter(m_states));
  }
  
  virtual Status event(Event& evt, Env& env) {
    InputModule::Status state = InputModule::Stop;
    if (not m_states.empty()) {
      state = m_states.front();
      m_states.pop_front();
    }
    return state;
  }
  
private:
  
  std::deque<psana::InputModule::Status> m_states;  
};

struct Fixture {
  
  Fixture(const InputModule::Status states[], int nstates) 
  {
    boost::shared_ptr<PSEnv::IExpNameProvider> expNameProvider;
    boost::shared_ptr<PSEnv::Env> env = boost::make_shared<PSEnv::Env>("", expNameProvider, "");
    boost::shared_ptr<InputModule> input = boost::make_shared<TestInputModule>(states, nstates);
    const std::vector<boost::shared_ptr<Module> > modules;
    evtLoop = boost::make_shared<EventLoop>(input, modules, env);
  }
  
  boost::shared_ptr<EventLoop> evtLoop;
};


}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_1 )
{
  InputModule::Status states[] = {
      InputModule::BeginRun,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::EndRun,
  };

  Fixture f(states, sizeof states/sizeof states[0]);

  EventLoop::value_type evt;
  
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::BeginJob);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::BeginRun);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::BeginCalibCycle);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::Event);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::Event);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::EndCalibCycle);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::EndRun);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::EndJob);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::None);
}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_2 )
{
  InputModule::Status states[] = {
      InputModule::BeginRun,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::EndRun,
  };

  Fixture f(states, sizeof states/sizeof states[0]);

  EventLoop::value_type evt;
  
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::BeginJob);
  f.evtLoop->putback(evt);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::BeginJob);

  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::BeginRun);
  f.evtLoop->putback(evt);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::BeginRun);

  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::BeginCalibCycle);
  f.evtLoop->putback(evt);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::BeginCalibCycle);

  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::Event);
  f.evtLoop->putback(evt);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::Event);

  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::Event);
  f.evtLoop->putback(evt);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::Event);

  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::EndCalibCycle);
  f.evtLoop->putback(evt);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::EndCalibCycle);

  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::EndRun);
  f.evtLoop->putback(evt);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::EndRun);

  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::EndJob);
  f.evtLoop->putback(evt);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::EndJob);

  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::None);
  f.evtLoop->putback(evt);
  evt = f.evtLoop->next();
  BOOST_CHECK_EQUAL(evt.first, EventLoop::None);

}

// ==============================================================

