#ifndef PSANA_EXAMPLES_DUMPBLD_H
#define PSANA_EXAMPLES_DUMPBLD_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpBld.
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

class DumpBld : public Module {
public:

  // Default constructor
  DumpBld (const std::string& name) ;

  // Destructor
  virtual ~DumpBld () ;

  /// Method which is called at the beginning of job
  virtual void beginJob(Event& evt, Env& env);

  /// Method which is called with event data
  virtual void event(Event& evt, Env& env);
  
protected:

private:
  
  Source m_ebeamSrc;
  Source m_cavSrc;
  Source m_feeSrc;
  Source m_ipimbSrc;
  Source m_pimSrc;

};

} // namespace psana_examples

#endif // PSANA_EXAMPLES_DUMPBLD_H
