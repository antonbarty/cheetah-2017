#ifndef PSANA_CONFIGURABLE_H
#define PSANA_CONFIGURABLE_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: Configurable.h 2476 2011-10-05 16:29:02Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class Configurable.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "ConfigSvc/ConfigSvc.h"

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
 *  @brief Class that provides a simplified interface to a framework's 
 *  configuration service. 
 *  
 *  This object can be used either as a base class or a member of other 
 *  classes. It does not define any virtual methods (even destructor)
 *  so make sure that classes which inherit it do it properly.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @see ConfigSvc::ConfigSvc
 *
 *  @version \$Id: Configurable.h 2476 2011-10-05 16:29:02Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andrei Salnikov
 */

class Configurable  {
public:

  /**
   *  @brief Constructor takes a name.
   *  
   *  It accept names in the format "Package.ClassName" or "Package.ClassName:InstanceName".
   *  The configuration will be read from a section corresponding to this name if present
   *  of from a section without instance name.
   *  
   *  @param[in] name Name of this configurable.
   */
  Configurable (const std::string& name) ;

  // Destructor
  ~Configurable();

  /**
   *  @brief Get the value of a single parameter, this method can be used for numeric types only.
   *  
   *  @param[in] param  Name of the parameter  
   *  @return Parameter value
   *  
   *  @throw ConfigSvc::ExceptionMissing thrown if parameter or section is not found
   */
  ConfigSvc::ConfigSvc::Result config(const std::string& param) const
  {
    ConfigSvc::ConfigSvc cfg;
    try {
      return cfg.get(name(), param);
    } catch (const ConfigSvc::ExceptionMissing& ex) {
      return cfg.get(className(), param);
    }
  }

  /**
   *  @brief Get the value of a single parameter as a string.
   *  
   *  @param[in] param  Name of the parameter  
   *  @return Parameter value
   *  
   *  @throw ConfigSvc::ExceptionMissing thrown if parameter or section is not found
   */
  std::string configStr(const std::string& param) const
  {
    ConfigSvc::ConfigSvc cfg;
    try {
      return cfg.getStr(name(), param);
    } catch (const ConfigSvc::ExceptionMissing& ex) {
      return cfg.getStr(className(), param);
    }
  }

  /**
   *  @brief Get the value of a single parameter, this method can be used for numeric types only.
   *  
   *  Returns default value if parameter is not found. 
   *  
   *  @param[in] param  Name of the parameter  
   *  @param[in] def    Default value to return if parameter is not there  
   *  @return Parameter value or default value
   */
  template <typename T>
  T config(const std::string& param, const T& def) const
  {
    ConfigSvc::ConfigSvc cfg;
    try {
      return cfg.get(name(), param);
    } catch (const ConfigSvc::ExceptionMissing& ex) {
      return cfg.get<T>(className(), param, def);
    }    
  }

  /**
   *  @brief Get the value of a single parameter as a string.
   *  
   *  Returns default value if parameter is not found. 
   *  
   *  @param[in] param  Name of the parameter  
   *  @param[in] def    Default value to return if parameter is not there
   *  @return Parameter value or default value
   */
  std::string configStr(const std::string& param, const std::string& def) const
  {
    ConfigSvc::ConfigSvc cfg;
    try {
      return cfg.getStr(name(), param);
    } catch (const ConfigSvc::ExceptionMissing& ex) {
      return cfg.getStr(className(), param, def);
    }    
  }

  /**
   *  @brief Get the value of a parameter as a sequence.
   *  
   *  @param[in] param  Name of the parameter  
   *  @return The object that is convertible to sequence type such as std::list<<std::string>
   *          or std::vector<int>
   *  
   *  @throw ConfigSvc::ExceptionMissing thrown if parameter or section is not found
   */
  ConfigSvc::ConfigSvc::ResultList configList(const std::string& param) const
  {
    ConfigSvc::ConfigSvc cfg;
    try {
      return cfg.getList(name(), param);
    } catch (const ConfigSvc::ExceptionMissing& ex) {
      return cfg.getList(className(), param);
    }
  }
  
  /**
   *  @brief Get the value of a parameter as a sequence.
   *  
   *  Returns default value if parameter is not found. 
   *  
   *  @param[in] param  Name of the parameter
   *  @param[in] def    Default value to return if parameter is not there
   *  @return The object that is convertible to sequence type such as std::list<<std::string>
   *          or std::vector<int>
   */
  template <typename T>
  std::list<T> configList(const std::string& param, const std::list<T>& def) const
  {
    ConfigSvc::ConfigSvc cfg;
    try {
      return cfg.getList(name(), param);
    } catch (const ConfigSvc::ExceptionMissing& ex) {
      return cfg.getList(className(), param, def);
    }
  }

  /// Get the full name of the object including class and instance name.
  const std::string& name() const { return m_name; }
  
  /// Get the class name of the object.
  const std::string& className() const { return m_className; }

protected:

private:

  // Data members
  std::string m_name;
  std::string m_className;

};

} // namespace psana

#endif // PSANA_CONFIGURABLE_H
