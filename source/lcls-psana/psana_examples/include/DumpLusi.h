#ifndef PSANA_EXAMPLES_DUMPLUSI_H
#define PSANA_EXAMPLES_DUMPLUSI_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: DumpLusi.h 1919 2011-05-21 12:04:08Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class DumpLusi.
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
 *  @version $Id: DumpLusi.h 1919 2011-05-21 12:04:08Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andrei Salnikov
 */

class DumpLusi : public Module {
public:

  // Default constructor
  DumpLusi (const std::string& name) ;

  // Destructor
  virtual ~DumpLusi () ;

  /// Method which is called at the beginning of the calibration cycle
  virtual void beginCalibCycle(Event& evt, Env& env);
  
  /// Method which is called with event data
  virtual void event(Event& evt, Env& env);
  
protected:

private:

  Source m_ipimbSrc;
  Source m_tmSrc;

};

} // namespace psana_examples

#endif // PSANA_EXAMPLES_DUMPLUSI_H
