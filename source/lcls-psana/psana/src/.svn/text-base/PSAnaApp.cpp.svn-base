//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class PSAnaApp...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/PSAnaApp.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
#include <unistd.h>
#include <boost/make_shared.hpp>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgFormatter.h"
#include "MsgLogger/MsgLogger.h"
#include "psana/Exceptions.h"
#include "psana/PSAna.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//
//  Application class
//

//----------------
// Constructors --
//----------------
PSAnaApp::PSAnaApp ( const std::string& appName )
  : AppUtils::AppBase( appName )
  , m_calibDirOpt( 'b', "calib-dir", "path", "calibration directory name, may include {exp} and {instr}", "" )
  , m_configOpt( 'c', "config", "path", "configuration file, def: psana.cfg", "" )
  , m_expNameOpt( 'e', "experiment", "string", "experiment name, format: XPP:xpp12311 or xpp12311", "" )
  , m_jobNameOpt( 'j', "job-name", "string", "job name, def: from input files", "" )
  , m_modulesOpt( 'm', "module", "name", "module name, more than one possible" )
  , m_maxEventsOpt( 'n', "num-events", "number", "maximum number of events to process, def: all", 0U )
  , m_skipEventsOpt( 's', "skip-events", "number", "number of events to skip, def: 0", 0U )
  , m_optionsOpt( 'o', "option", "string", "configuration options, format: section.option[=value]" )
  , m_files( "data-file",   "file name(s) with input data", std::list<std::string>() )
{
  addOption( m_calibDirOpt ) ;
  addOption( m_configOpt ) ;
  addOption( m_expNameOpt ) ;
  addOption( m_jobNameOpt ) ;
  addOption( m_modulesOpt ) ;
  addOption( m_maxEventsOpt ) ;
  addOption( m_skipEventsOpt ) ;
  addOption( m_optionsOpt ) ;
  addArgument( m_files ) ;
}

//--------------
// Destructor --
//--------------
PSAnaApp::~PSAnaApp ()
{
}

/**
 *  Run the application, accepts arguments as vector of strings which
 *  should contain the same values as regular argv (first element must be
 *  application name).
 */
int
PSAnaApp::run(const std::vector<std::string>& argv)
{
  // copy arguments
  size_t size = 0;
  for (std::vector<std::string>::const_iterator it = argv.begin(); it != argv.end(); ++ it) {
    size += it->size() + 1;
  }

  char* buf = new char[size];

  std::vector<char*> cargv;
  cargv.reserve(argv.size()+1);
  char* p = buf;
  for (std::vector<std::string>::const_iterator it = argv.begin(); it != argv.end(); ++ it) {
    cargv.push_back(p);
    p = std::copy(it->begin(), it->end(), p);
    *p++ = '\0';
  }
  cargv.push_back(0);

  // call standard method
  int code = this->run(argv.size(), &cargv[0]);

  // cleanup
  delete [] buf;

  return code;
}

/**
 *  Method called before runApp, can be overriden in subclasses.
 *  Usually if you override it, call base class method too.
 */
int
PSAnaApp::preRunApp ()
{
  AppBase::preRunApp();

  // use different formatting for messages
  const char* fmt = "[%(level):%(logger)] %(message)" ;
  const char* errfmt = "[%(level):%(time):%(file):%(line)] %(message)" ;
  const char* trcfmt = "[%(level):%(time):%(logger)] %(message)" ;
  const char* dbgfmt = errfmt ;
  MsgLogger::MsgFormatter::addGlobalFormat ( fmt ) ;
  MsgLogger::MsgFormatter::addGlobalFormat ( MsgLogger::MsgLogLevel::debug, dbgfmt ) ;
  MsgLogger::MsgFormatter::addGlobalFormat ( MsgLogger::MsgLogLevel::trace, trcfmt ) ;
  MsgLogger::MsgFormatter::addGlobalFormat ( MsgLogger::MsgLogLevel::warning, errfmt ) ;
  MsgLogger::MsgFormatter::addGlobalFormat ( MsgLogger::MsgLogLevel::error, errfmt ) ;
  MsgLogger::MsgFormatter::addGlobalFormat ( MsgLogger::MsgLogLevel::fatal, errfmt ) ;

  return 0;
}

/**
 *  Main method which runs the whole application
 */
int
PSAnaApp::runApp ()
{
  // if -c is not specified the try to read psana.cfg (only if present)
  std::string cfgFile = m_configOpt.value();
  if (not m_configOpt.valueChanged()) {
    if (access("psana.cfg", R_OK) == 0) {
      cfgFile = "psana.cfg";
    }
  }

  std::map<std::string, std::string> options;

  // command-line -m options override config file values
  if (not m_modulesOpt.value().empty()) {
    std::string modlist;
    const std::list<std::string>& modules = m_modulesOpt.value();
    for(std::list<std::string>::const_iterator it = modules.begin(); it != modules.end(); ++ it) {
      if (not modlist.empty()) modlist += ' ';
      modlist += *it;
    }
    MsgLogRoot(trace, "set module list to '" << modlist << "'");
    options["psana.modules"] = modlist;
  }

  // set instrument and experiment names if specified
  if (not m_expNameOpt.value().empty()) {
      std::string instrName;
      std::string expName = m_expNameOpt.value();
      std::string::size_type pos = expName.find(':');
      if (pos == std::string::npos) {
          instrName = expName.substr(0, 3);
          boost::to_upper(instrName);
      } else {
          instrName = expName.substr(0, pos);
          expName.erase(0, pos+1);
      }
      MsgLogRoot(debug, "cmd line: instrument = " << instrName << " experiment = " << expName);
      options["psana.instrument"] = instrName;
      options["psana.experiment"] = expName;
  }

  // set event numbers
  if (m_maxEventsOpt.value()) {
      options["psana.events"] = boost::lexical_cast<std::string>(m_maxEventsOpt.value());
  }
  if (m_skipEventsOpt.value()) {
      options["psana.skip-events"] = boost::lexical_cast<std::string>(m_skipEventsOpt.value());
  }

  // set calib dir name if specified
  if (not m_calibDirOpt.value().empty()) {
    options["psana.calib-dir"] = m_calibDirOpt.value();
  }

  // now copy all -o options, they may override existing options
  typedef AppUtils::AppCmdOptList<std::string>::const_iterator OptIter;
  for (OptIter it = m_optionsOpt.begin(); it != m_optionsOpt.end(); ++ it) {
    std::string optname = *it;
    std::string optval;
    std::string::size_type p = optname.find('=');
    if (p != std::string::npos) {
      optval = std::string(optname, p+1);
      optname.erase(p);
    }
    options[optname] = optval;
  }

  // list of inputs
  std::vector<std::string> input(m_files.begin(), m_files.end());

  // Instantiate framework
  PSAna fwk(cfgFile, options);

  // check that we have at least one module
  if (fwk.modules().empty()) {
    MsgLogRoot(error, "no analysis modules specified");
    return 2;
  }

  // get data source
  DataSource dataSource = fwk.dataSource(input);
  if (dataSource.empty()) return 2;

  // get event iterator
  EventIter iter = dataSource.events();

  // loop from begin to end
  while (boost::shared_ptr<PSEvt::Event> evt = iter.next()) {
    // nothing to do here
  }

  // return 0 on success, other values for error (like main())
  return 0 ;
}

} // namespace psana
