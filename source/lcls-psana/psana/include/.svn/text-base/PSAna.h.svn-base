#ifndef PSANA_PSANA_H
#define PSANA_PSANA_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class PSAna.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <map>
#include <vector>
#include <boost/utility.hpp>

//----------------------
// Base Class Headers --
//----------------------


//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psana/Context.h"
#include "psana/DataSource.h"

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
 *  @brief PSana framework class.
 *
 *  This class makes instances of the psana framework. For now there could be
 *  just one single instance, attempt to instantiate another instance will
 *  likely lead to exception.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class PSAna : boost::noncopyable {
public:

  /**
   *  @brief Make framework instance.
   *
   *  Framework constructor takes the name of the configuration file which
   *  can be empty and the set of the configuration options. Option provided
   *  via mapping, keys have format "section.param", values are strings.
   *  If section is not specified then it defaults to "psana".
   *  Options provided in constructor override options specified in
   *  configuration file.
   *
   *  @param[in]  config    Name of the configuration file, typically "psana.cfg".
   *  @param[in]  options   Other configuration options
   *
   */
  PSAna(const std::string& config, const std::map<std::string, std::string>& options);

  // Destructor
  ~PSAna () ;

  /**
   *  @brief Get the list of modules.
   */
  std::vector<std::string> modules();

  /**
   *  @brief Create data source instance for the set of input files/datasets.
   *
   *  This method can be called multiple times with different (or same) set of
   *  inputs. If list of inputs is empty then inputs from configuration file
   *  are used.
   *
   *  @param[in] input   List of inputs which can include files names, dataset names, etc.
   *  @return Instance of the data source class.
   */
  DataSource dataSource(const std::vector<std::string>& input);

protected:

private:

  Context::context_t m_context;                       ///< context (id) of this framework instance
  std::vector<boost::shared_ptr<Module> > m_modules;  ///< list of user modules

};

} // namespace psana

#endif // PSANA_PSANA_H
