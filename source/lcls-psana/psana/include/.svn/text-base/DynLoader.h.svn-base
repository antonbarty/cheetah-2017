#ifndef PSANA_DYNLOADER_H
#define PSANA_DYNLOADER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DynLoader.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <boost/shared_ptr.hpp>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psana/Module.h"
#include "psana/InputModule.h"

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
 *  @brief Class which can load modules from dynamic libraries.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version \$Id$
 *
 *  @author Andrei Salnikov
 */

class DynLoader  {
public:

  /**
   *  @brief Load one user module.
   *
   *  The name of the module has a format [Language:][Package.]Class[:name].
   *  Accepted languages are "c++", "python", "py" (same as "python).
   */
  boost::shared_ptr<Module> loadModule(const std::string& name) const;
  
  /**
   *  @brief Load one input module.
   *
   *  The name of the module has a format [Package.]Class[:name]
   */
  boost::shared_ptr<InputModule> loadInputModule(const std::string& name) const;
  
protected:

  /**
   *  @brief Load the library for a package.
   *  
   *  @param[in] packageName  Package name.
   *  @return Library handle.
   *
   *  @throw ExceptionDlerror
   */
  void* loadPackageLib(const std::string& packageName) const;
  
  /**
   *  @brief Load the library and find factory symbol
   *  
   *  @param[in] className String in the format Package.Class
   *  @param[in] factory Prefix for factory function name, like "_psana_module_"
   *  @return pointer to factory function.
   *  
   *  @throw ExceptionModuleName
   *  @throw ExceptionDlerror
   */
  void* loadFactoryFunction(const std::string& className, const std::string& factory) const;
  
private:

  /**
   *  @brief Load one user module for the given language. The name of the module has a format 
   *  [Package.]Class[:name]
   */
  boost::shared_ptr<Module> loadModule(const std::string& name, const std::string& language) const;

};

} // namespace psana

#endif // PSANA_DYNLOADER_H
