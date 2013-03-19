//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Run...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/Run.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psana/EventLoop.h"

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
Run::Run ()
  : m_evtLoop()
  , m_run(-1)
{
}

// Constructor takes event loop object
Run::Run(const boost::shared_ptr<EventLoop>& evtLoop, int run)
  : m_evtLoop(evtLoop)
  , m_run(run)
{
}

//--------------
// Destructor --
//--------------
Run::~Run ()
{
}

/// Get environment object, cannot be called for "null" source
PSEnv::Env&
Run::env() const
{
  return m_evtLoop->env();
}

} // namespace psana
