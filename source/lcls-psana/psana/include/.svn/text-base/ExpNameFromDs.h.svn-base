#ifndef PSANA_EXPNAMEFROMDS_H
#define PSANA_EXPNAMEFROMDS_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ExpNameFromDs.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <list>
#include <vector>

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
 *  @brief Experiment name provider which extracts experiment name from XTC
 *  file names.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class ExpNameFromDs : public PSEnv::IExpNameProvider {
public:

  /// Constructor takes the list of input file names
  ExpNameFromDs(const std::vector<std::string>& files);

  // Destructor
  virtual ~ExpNameFromDs();

  /// Returns instrument name
  virtual const std::string& instrument() const { return m_instr; }

  /// Returns experiment name
  virtual const std::string& experiment() const { return m_exp; }

  /// Returns experiment number or 0
  virtual unsigned expNum() const { return m_expNum; }

protected:

private:
  
  std::string m_instr;
  std::string m_exp;
  unsigned m_expNum;

};

} // namespace psana

#endif // PSANA_EXPNAMEFROMDS_H
