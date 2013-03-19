//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: InputIterTest.cpp 3781 2012-06-11 01:01:41Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Test suite case for the InputIterTest.
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
#include "psana/InputIter.h"
#include "psana/InputModule.h"
#include "PSEnv/Env.h"

using namespace psana ;

#define BOOST_TEST_MODULE InputIterTest
#include <boost/test/included/unit_test.hpp>

/**
 * Simple test suite for class InputIter.
 * See http://www.boost.org/doc/libs/1_36_0/libs/test/doc/html/index.html
 */

namespace {

// Implementation of the InputModulle which generates sequences of events
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
  
  Fixture(const InputModule::Status states[], int nstates, InputIter::EventType expected[], unsigned nexpected) 
  {
    boost::shared_ptr<PSEnv::IExpNameProvider> expNameProvider;
    boost::shared_ptr<PSEnv::Env> env = boost::make_shared<PSEnv::Env>("", expNameProvider, "");
    boost::shared_ptr<InputModule> input = boost::make_shared<TestInputModule>(states, nstates);
    iter = boost::make_shared<InputIter>(input, env);
    std::copy(expected, expected+nexpected, std::back_inserter(exp));

  }
  
  std::vector<InputIter::EventType> readAll() 
  {
    std::vector<InputIter::EventType> result;
    InputIter::value_type val;
    do {
      val = iter->next();
      result.push_back(val.first);
    } while (val.first != InputIter::None);
    return result;
  }

  bool checkResult()
  {
    std::vector<InputIter::EventType> res = readAll();
    
    std::cout << "expected: ";
    std::copy(exp.begin(), exp.end(), std::ostream_iterator<InputIter::EventType>(std::cout, " "));
    std::cout << "\n";
    std::cout << "result  : ";
    std::copy(res.begin(), res.end(), std::ostream_iterator<InputIter::EventType>(std::cout, " "));
    std::cout << "\n";

    return res == exp;
  }

  boost::shared_ptr<InputIter> iter;
  std::vector<InputIter::EventType> exp;
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
  InputIter::EventType expected[] = {
      InputIter::BeginJob,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::EndJob,
      InputIter::None,
  };
  
  Fixture f(states, sizeof states/sizeof states[0], expected, sizeof expected/sizeof expected[0]);
  BOOST_CHECK(f.checkResult());
}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_2 )
{
  InputModule::Status states[] = {
      InputModule::BeginRun,
      InputModule::Skip,
      InputModule::BeginCalibCycle,
      InputModule::Skip,
      InputModule::DoEvent,
      InputModule::Skip,
      InputModule::DoEvent,
      InputModule::Skip,
      InputModule::EndCalibCycle,
      InputModule::Skip,
      InputModule::EndRun,
  };
  InputIter::EventType expected[] = {
      InputIter::BeginJob,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::EndJob,
      InputIter::None,
  };
  
  
  Fixture f(states, sizeof states/sizeof states[0], expected, sizeof expected/sizeof expected[0]);
  BOOST_CHECK(f.checkResult());
}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_3 )
{
  InputModule::Status states[] = {
      InputModule::BeginRun,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::EndRun,
  };
  InputIter::EventType expected[] = {
      InputIter::BeginJob,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::EndJob,
      InputIter::None,
  };
  
  Fixture f(states, sizeof states/sizeof states[0], expected, sizeof expected/sizeof expected[0]);
  BOOST_CHECK(f.checkResult());
}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_4 )
{
  InputModule::Status states[] = {
      InputModule::BeginRun,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::EndRun,
      InputModule::BeginRun,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::EndRun,
  };
  InputIter::EventType expected[] = {
      InputIter::BeginJob,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::EndJob,
      InputIter::None,
  };
  
  Fixture f(states, sizeof states/sizeof states[0], expected, sizeof expected/sizeof expected[0]);
  BOOST_CHECK(f.checkResult());
}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_5 )
{
  InputModule::Status states[] = {
      InputModule::BeginRun,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
//      InputModule::EndCalibCycle,
//      InputModule::EndRun,
      InputModule::BeginRun,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::EndRun,
  };
  InputIter::EventType expected[] = {
      InputIter::BeginJob,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::EndJob,
      InputIter::None,
  };
  
  Fixture f(states, sizeof states/sizeof states[0], expected, sizeof expected/sizeof expected[0]);
  BOOST_CHECK(f.checkResult());
}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_6 )
{
  InputModule::Status states[] = {
      InputModule::BeginRun,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
//      InputModule::EndCalibCycle,
//      InputModule::EndRun,
  };
  InputIter::EventType expected[] = {
      InputIter::BeginJob,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::EndJob,
      InputIter::None,
  };
  
  Fixture f(states, sizeof states/sizeof states[0], expected, sizeof expected/sizeof expected[0]);
  BOOST_CHECK(f.checkResult());
}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_7 )
{
  InputModule::Status states[] = {
//      InputModule::BeginRun,
//      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::EndRun,
  };
  InputIter::EventType expected[] = {
      InputIter::BeginJob,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::EndJob,
      InputIter::None,
  };
  
  Fixture f(states, sizeof states/sizeof states[0], expected, sizeof expected/sizeof expected[0]);
  BOOST_CHECK(f.checkResult());
}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_8 )
{
  InputModule::Status states[] = {
      InputModule::BeginRun,
      InputModule::BeginCalibCycle,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::BeginRun,
      InputModule::DoEvent,
      InputModule::EndCalibCycle,
      InputModule::EndRun,
  };
  InputIter::EventType expected[] = {
      InputIter::BeginJob,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::BeginRun,
      InputIter::BeginCalibCycle,
      InputIter::Event,
      InputIter::EndCalibCycle,
      InputIter::EndRun,
      InputIter::EndJob,
      InputIter::None,
  };
  
  Fixture f(states, sizeof states/sizeof states[0], expected, sizeof expected/sizeof expected[0]);
  BOOST_CHECK(f.checkResult());
}

// ==============================================================

