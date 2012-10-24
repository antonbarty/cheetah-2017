#ifndef PSANA_EXAMPLES_DUMPEPICS_H
#define PSANA_EXAMPLES_DUMPEPICS_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpEpics.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

//----------------------
// Base Class Headers --
//----------------------
#include "psana/Module.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana_examples {

/**
 *  @brief Example module for accessing EPICS data from psana
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andrei Salnikov
 */

class DumpEpics : public Module {
public:

  // Default constructor
  DumpEpics (const std::string& name) ;

  // Destructor
  virtual ~DumpEpics () ;

  /// Method which is called with event data
  virtual void event(Event& evt, Env& env);
  
protected:

private:

  // Data members

};

} // namespace psana_examples

#endif // PSANA_EXAMPLES_DUMPEPICS_H
