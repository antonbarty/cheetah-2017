//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class PrintEventId...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/PrintEventId.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "PSEvt/EventId.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana;
PSANA_MODULE_FACTORY(PrintEventId)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
PrintEventId::PrintEventId (const std::string& name)
  : Module(name)
{
}

//--------------
// Destructor --
//--------------
PrintEventId::~PrintEventId ()
{
}

/// Method which is called once at the beginning of the job
void 
PrintEventId::beginJob(Event& evt, Env& env)
{
  MsgLog(name(), info, "in beginJob()");
  printId(evt);
}

/// Method which is called at the beginning of the run
void 
PrintEventId::beginRun(Event& evt, Env& env)
{
  MsgLog(name(), info, "in beginRun()");
  printId(evt);
}

/// Method which is called at the beginning of the calibration cycle
void 
PrintEventId::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), info, "in beginCalibCycle()");
  printId(evt);
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
PrintEventId::event(Event& evt, Env& env)
{
  printId(evt);
}
  
/// Method which is called at the end of the calibration cycle
void 
PrintEventId::endCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), info, "in endCalibCycle()");
  printId(evt);
}

/// Method which is called at the end of the run
void 
PrintEventId::endRun(Event& evt, Env& env)
{
  MsgLog(name(), info, "in endRun()");
  printId(evt);
}

/// Method which is called once at the end of the job
void 
PrintEventId::endJob(Event& evt, Env& env)
{
  MsgLog(name(), info, "in endJob()");
  printId(evt);
}

/// fetch and print event ID
void
PrintEventId::printId(Event& evt)
{
  // get event ID
  shared_ptr<EventId> eventId = evt.get();
  if (not eventId.get()) {
    MsgLog(name(), info, "event ID not found");
  } else {
    MsgLog(name(), info, "event ID: " << *eventId);
  }
}

} // namespace psana
