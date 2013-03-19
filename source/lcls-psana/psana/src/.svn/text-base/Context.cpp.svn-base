//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Context...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/Context.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace {

  psana::Context::context_t g_next = 0;
  psana::Context::context_t g_current = 0;

}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

// generate new unique context value, never returns 0 value.
psana::Context::context_t
psana::Context::generate()
{
  return ++::g_next;
}

// Set the context
void
psana::Context::set(psana::Context::context_t ctx)
{
  ::g_current = ctx;
}

// Get current context, returns 0 if context was not set yet
psana::Context::context_t
psana::Context::get()
{
  return ::g_current;
}
