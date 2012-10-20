#ifndef PSANA_EXAMPLES_DUMPCSPAD2X2_H
#define PSANA_EXAMPLES_DUMPCSPAD2X2_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpCsPad2x2.
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

class DumpCsPad2x2 : public Module {
public:

  // Default constructor
  DumpCsPad2x2 (const std::string& name) ;

  // Destructor
  virtual ~DumpCsPad2x2 () ;

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

#endif // PSANA_EXAMPLES_DUMPCSPAD2X2_H
