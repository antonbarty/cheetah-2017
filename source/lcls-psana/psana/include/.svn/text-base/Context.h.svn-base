#ifndef PSANA_CONTEXT_H
#define PSANA_CONTEXT_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Context.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

//----------------------
// Base Class Headers --
//----------------------


//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana {

/// @addtogroup psana

/**
 *  @ingroup psana
 *
 *  @brief Methods dealing with Framework context.
 *
 *  There could be several instances of the psana framework (including
 *  separate sets of modules and separate configurations) in one
 *  application. Some services need to know which framework instance
 *  they serve, for example configuration service needs to provide
 *  different set of parameters for modules in different frameworks.
 *  To simplify implementation of user modules we want to avoid passing
 *  information about current framework instance from framework down to
 *  modules. Instead we use concept of context to implement indirect
 *  notification of the clients about current context.
 *
 *  The context is implemented as a global object keeping some abstract
 *  values that are unique for different frameworks (index of a framework
 *  instance could be used for example). This global object is set by
 *  framework instance during initialization. Any object that needs to
 *  know context should copy its value and use it later to identify
 *  framework instance. Constructor of a module is one good place where
 *  the context can be checked. After framework initialization it is never
 *  guaranteed that context could be updated.
 *
 *  NOTE: current implementation not thread safe and should not be used
 *  from multiple threads.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

namespace Context  {

  /// Abstraction of a context type
  typedef int context_t;

  /// generate new unique context value, never returns 0 value.
  context_t generate();

  /// Set the context
  void set(context_t ctx);

  /// Get current context, returns 0 if context was not set yet
  context_t get();

} // namespace Context
} // namespace psana

#endif // PSANA_CONTEXT_H
