#ifndef CHUCK_ANA_PKG_CHUCK_ANA_MOD_H
#define CHUCK_ANA_PKG_CHUCK_ANA_MOD_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class chuck_ana_mod.
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

namespace chuck_ana_pkg {

/// @addtogroup chuck_ana_pkg

/**
 *  @ingroup chuck_ana_pkg
 *
 *  @brief Example module class for psana
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version \$Id$
 *
 *  @author Chunhong Yoon
 */

class chuck_ana_mod : public Module {
public:

  // Default constructor
  chuck_ana_mod (const std::string& name) ;

  // Destructor
  virtual ~chuck_ana_mod () ;
 
  // Method called at the beginning of the calibration cycle
  virtual void beginCalibCycle(Event& evt, Env& env);

  /// Method which is called with event data, this is the only required 
  /// method, all other methods are optional
  virtual void event(Event& evt, Env& env);

private:

  // Data members
  std::string m_key;
  Source m_src;
  Source m_srcEvr;
  Source m_srcBeam;
  Source m_srcFee;
  Source m_srcCav;
  Source m_srcAcq;
  Source m_srcCam;
};

} // namespace chuck_ana_pkg

#endif // CHUCK_ANA_PKG_CHUCK_ANA_MOD_H
