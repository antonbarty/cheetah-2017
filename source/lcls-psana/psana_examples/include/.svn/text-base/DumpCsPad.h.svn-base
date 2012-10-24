#ifndef PSANA_EXAMPLES_DUMPCSPAD_H
#define PSANA_EXAMPLES_DUMPCSPAD_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpCsPad.
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
 *  @brief Example module class for psana
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @see AdditionalClass
 *
 *  @version $Id$
 *
 *  @author Andrei Salnikov
 */

class DumpCsPad : public Module {
public:

  // Default constructor
  DumpCsPad (const std::string& name) ;

  // Destructor
  virtual ~DumpCsPad () ;

  /// Method which is called at the beginning of the calibration cycle
  virtual void beginCalibCycle(Event& evt, Env& env);
  
  /// Method which is called with event data
  virtual void event(Event& evt, Env& env);

protected:

private:

  // Data members, this is for example purposes only
  std::string m_key;
  Source m_src;

};

} // namespace psana_examples

#endif // PSANA_EXAMPLES_DUMPCSPAD_H
