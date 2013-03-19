//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Scan...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/Scan.h"

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
Scan::Scan ()
  : m_evtLoop()
{
}

// Constructor takes event loop object
Scan::Scan(const boost::shared_ptr<EventLoop>& evtLoop)
  : m_evtLoop(evtLoop)
{
}

//--------------
// Destructor --
//--------------
Scan::~Scan ()
{
}

/// Get environment object, cannot be called for "null" source
PSEnv::Env&
Scan::env() const
{
  return m_evtLoop->env();
}


} // namespace psana
