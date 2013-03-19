//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Exceptions...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/Exceptions.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <cerrno>
#include <string.h>
#include <dlfcn.h>

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

Exception::Exception( const ErrSvc::Context& ctx, const std::string& what )
  : ErrSvc::Issue( ctx, "psana::Exception: " + what )
{
}

ExceptionModuleName::ExceptionModuleName ( const ErrSvc::Context& ctx, const std::string& module )
  : Exception( ctx, "invalid module name: " + module)
{  
}

ExceptionErrno::ExceptionErrno ( const ErrSvc::Context& ctx, const std::string& what )
  : Exception( ctx, what + ": " + strerror(errno) )
{
}

ExceptionDlerror::ExceptionDlerror ( const ErrSvc::Context& ctx, const std::string& what )
  : Exception( ctx, what + ": " + dlerror() )
{
}

ExceptionAbort::ExceptionAbort(const ErrSvc::Context& ctx, const std::string& what)
  : Exception(ctx, what)
{
}

} // namespace psana
