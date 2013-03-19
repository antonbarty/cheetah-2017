#ifndef PSANA_PSANAAPP_H
#define PSANA_PSANAAPP_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class PSAnaApp.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <vector>
#include <stack>
#include <boost/shared_ptr.hpp>

//----------------------
// Base Class Headers --
//----------------------
#include "AppUtils/AppBase.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "AppUtils/AppCmdArgList.h"
#include "AppUtils/AppCmdOpt.h"
#include "AppUtils/AppCmdOptList.h"
#include "psana/Module.h"
#include "PSEnv/Env.h"
#include "PSEvt/Event.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana {

/// @addtogroup psana

/**
 *  @ingroup psana
 *
 *  @brief Application calss for psana.
 *
 *  This class is a whole application, you can run psana by making an
 *  instance of this class and passing arguments to its run() method.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class PSAnaApp : public AppUtils::AppBase {
public:

  // Constructor
  explicit PSAnaApp(const std::string& appName = "psana") ;

  // destructor
  virtual ~PSAnaApp () ;

  /**
   *  Run the application, accepts regular argc and argv.
   */
  int run(int argc, char** argv) { return AppUtils::AppBase::run(argc, argv); }

  /**
   *  Run the application, accepts arguments as vector of strings which
   *  should contain the same values as regular argv (first element must be
   *  application name).
   */
  int run(const std::vector<std::string>& argv);
  
protected :

  /**
   *  Method called before runApp, can be overriden in subclasses.
   *  Usually if you override it, call base class method too.
   */
  virtual int preRunApp () ;

  /**
   *  Main method which runs the whole application
   */
  virtual int runApp () ;

private:

  // more command line options and arguments
  AppUtils::AppCmdOpt<std::string> m_calibDirOpt ;
  AppUtils::AppCmdOpt<std::string> m_configOpt ;
  AppUtils::AppCmdOpt<std::string> m_expNameOpt ;
  AppUtils::AppCmdOpt<std::string> m_jobNameOpt ;
  AppUtils::AppCmdOptList<std::string> m_modulesOpt;
  AppUtils::AppCmdOpt<unsigned> m_maxEventsOpt ;
  AppUtils::AppCmdOpt<unsigned> m_skipEventsOpt ;
  AppUtils::AppCmdOptList<std::string> m_optionsOpt;
  AppUtils::AppCmdArgList<std::string>  m_files;
};

} // namespace psana

#endif // PSANA_PSANAAPP_H
