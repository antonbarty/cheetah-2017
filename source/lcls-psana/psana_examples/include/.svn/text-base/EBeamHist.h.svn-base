#ifndef PSANA_EXAMPLES_EBEAMHIST_H
#define PSANA_EXAMPLES_EBEAMHIST_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class EBeamHist.
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

class EBeamHist : public Module {
public:

  // Default constructor
  EBeamHist (const std::string& name) ;

  // Destructor
  virtual ~EBeamHist () ;

  /// Method which is called once at the beginning of the job
  virtual void beginJob(Event& evt, Env& env);
  
  /// Method which is called with event data
  virtual void event(Event& evt, Env& env);
  
protected:

private:

  // Data members
  
  Source m_ebeamSrc;
  PSHist::H1* m_ebeamHisto;
  PSHist::H1* m_chargeHisto;

};

} // namespace psana_examples

#endif // PSANA_EXAMPLES_EBEAMHIST_H
