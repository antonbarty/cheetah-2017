//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Module...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/Module.h"

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
Module::Module (const std::string& name, bool observeAllEvents)
  : Configurable(name)
  , m_status(OK)
  , m_observeAllEvents(observeAllEvents)
{
}

//--------------
// Destructor --
//--------------
Module::~Module ()
{
}

// Method which is called once at the beginning of the job
void 
Module::beginJob(Event& evt, Env& env)
{
}

// Method which is called at the beginning of the run
void 
Module::beginRun(Event& evt, Env& env)
{
}

// Method which is called at the beginning of the calibration cycle
void 
Module::beginCalibCycle(Event& evt, Env& env)
{
}

// Method which is called at the end of the calibration cycle
void 
Module::endCalibCycle(Event& evt, Env& env)
{
}

// Method which is called at the end of the run
void 
Module::endRun(Event& evt, Env& env)
{
}

// Method which is called once at the end of the job
void 
Module::endJob(Event& evt, Env& env)
{
}


// formatting for enum
std::ostream&
operator<<(std::ostream& out, Module::Status stat)
{
  const char* str = "???";
  switch (stat) {
  case Module::OK:
    str = "OK";
    break;
  case Module::Skip:
    str = "Skip";
    break;
  case Module::Stop:
    str = "Stop";
    break;
  case Module::Abort:
    str = "Abort";
    break;
  }
  return out << str;
}

} // namespace psana
