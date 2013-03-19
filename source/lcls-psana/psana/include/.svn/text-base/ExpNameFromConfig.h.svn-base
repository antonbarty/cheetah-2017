#ifndef PSANA_EXPNAMEFROMCONFIG_H
#define PSANA_EXPNAMEFROMCONFIG_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ExpNameFromConfig.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

//----------------------
// Base Class Headers --
//----------------------
#include "PSEnv/IExpNameProvider.h"

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

/**
 *  @ingroup psana
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @see AdditionalClass
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class ExpNameFromConfig : public PSEnv::IExpNameProvider {
public:

  /**
   *  Create provider instance
   *
   *  @param[in] instr   Instrument name
   *  @param[in] exp     Experiemnt name
   */
  ExpNameFromConfig(const std::string& instr, const std::string& exp) ;

  // Destructor
  virtual ~ExpNameFromConfig () ;

  /// Returns instrument name
  virtual const std::string& instrument() const { return m_instr; }

  /// Returns experiment name
  virtual const std::string& experiment() const { return m_exp; }

  /// Returns experiment number or 0
  virtual unsigned expNum() const { return m_expNum; }

protected:

private:

  std::string m_instr;  ///< Name of instrument
  std::string m_exp;    ///< Name of experiment
  unsigned m_expNum;    ///< Experiment number

};

} // namespace psana

#endif // PSANA_EXPNAMEFROMCONFIG_H
