#ifndef PSANA_EXAMPLES_DUMPACQIRIS_H
#define PSANA_EXAMPLES_DUMPACQIRIS_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpAcqiris.
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
 *  @version $Id$
 *
 *  @author Andrei Salnikov
 */

class DumpAcqiris : public Module {
public:

  // Default constructor
  DumpAcqiris (const std::string& name) ;

  // Destructor
  virtual ~DumpAcqiris () ;

  /// Method which is called at the beginning of the calibration cycle
  virtual void beginCalibCycle(Event& evt, Env& env);
  
  /// Method which is called with event data
  virtual void event(Event& evt, Env& env);
  
protected:

private:

  // Data members, this is for example purposes only
  Pds::Src m_src;

};

} // namespace psana_examples

#endif // PSANA_EXAMPLES_DUMPACQIRIS_H
