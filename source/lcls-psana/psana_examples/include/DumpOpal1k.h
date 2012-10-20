#ifndef PSANA_EXAMPLES_DUMPOPAL1K_H
#define PSANA_EXAMPLES_DUMPOPAL1K_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: DumpOpal1k.h 1919 2011-05-21 12:04:08Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class DumpOpal1k.
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
 *  @version $Id: DumpOpal1k.h 1919 2011-05-21 12:04:08Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andrei Salnikov
 */

class DumpOpal1k : public Module {
public:

  // Default constructor
  DumpOpal1k (const std::string& name) ;

  // Destructor
  virtual ~DumpOpal1k () ;

  /// Method which is called at the beginning of the calibration cycle
  virtual void beginCalibCycle(Event& evt, Env& env);
  
  /// Method which is called with event data
  virtual void event(Event& evt, Env& env);
  
protected:

private:

  Source m_src;

};

} // namespace psana_examples

#endif // PSANA_EXAMPLES_DUMPOPAL1K_H
