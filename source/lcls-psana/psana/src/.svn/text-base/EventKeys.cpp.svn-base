//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class EventKeys...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/EventKeys.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana;
PSANA_MODULE_FACTORY(EventKeys)

namespace {
  
  const char* logger = "EventKeys"; 
  
  void printKeys(std::ostream& out, const std::list<EventKey>& keys)
  {
    for (std::list<EventKey>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
      std::cout << "  " << *it << '\n';
    }
  }
}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
EventKeys::EventKeys (const std::string& name)
  : Module(name)
{
}

//--------------
// Destructor --
//--------------
EventKeys::~EventKeys ()
{
}

/// Method which is called once at the beginning of the job
void 
EventKeys::beginJob(Event& evt, Env& env)
{
  MsgLog(logger, info, name() << ": in beginJob()");
  
  std::cout << "Config keys:\n";
  ::printKeys(std::cout, env.configStore().keys());
  std::cout << "Calib keys:\n";
  ::printKeys(std::cout, env.calibStore().keys());
  std::cout << "Event keys:\n";
  ::printKeys(std::cout, evt.keys());
}

/// Method which is called at the beginning of the run
void 
EventKeys::beginRun(Event& evt, Env& env)
{
  MsgLog(logger, info, name() << ": in beginRun()");
  
  std::cout << "Config keys:\n";
  ::printKeys(std::cout, env.configStore().keys());
  std::cout << "Calib keys:\n";
  ::printKeys(std::cout, env.calibStore().keys());
  std::cout << "Event keys:\n";
  ::printKeys(std::cout, evt.keys());
}

/// Method which is called at the beginning of the calibration cycle
void 
EventKeys::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(logger, info, name() << ": in beginCalibCycle()");
  
  std::cout << "Config keys:\n";
  ::printKeys(std::cout, env.configStore().keys());
  std::cout << "Calib keys:\n";
  ::printKeys(std::cout, env.calibStore().keys());
  std::cout << "Event keys:\n";
  ::printKeys(std::cout, evt.keys());
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
EventKeys::event(Event& evt, Env& env)
{
  std::cout << "Event keys:\n";
  ::printKeys(std::cout, evt.keys());
}

/// Method which is called at the end of the calibration cycle (scan)
void
EventKeys::endCalibCycle(Event& evt, Env& env)
{
  MsgLog(logger, info, name() << ": in endCalibCycle()");

  std::cout << "Event keys:\n";
  ::printKeys(std::cout, evt.keys());
}

/// Method which is called at the end of the run
void
EventKeys::endRun(Event& evt, Env& env)
{
  MsgLog(logger, info, name() << ": in endRun()");

  std::cout << "Event keys:\n";
  ::printKeys(std::cout, evt.keys());
}

/// Method which is called once at the end of the job
void
EventKeys::endJob(Event& evt, Env& env)
{
  MsgLog(logger, info, name() << ": in endJob()");

  std::cout << "Event keys:\n";
  ::printKeys(std::cout, evt.keys());
}


} // namespace psana
