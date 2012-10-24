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
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "ConfigSvc/ConfigSvc.h"
#include "ConfigSvc/ConfigSvcImplFile.h"
#include "MsgLogger/MsgFormatter.h"
#include "MsgLogger/MsgLogger.h"
#include "psana/DynLoader.h"
#include "psana/Exceptions.h"
#include "psana/ExpNameFromConfig.h"
#include "psana/ExpNameFromXtc.h"
#include "PSEvt/ProxyDict.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace fs = boost::filesystem ;

namespace {

  enum FileType { Unknown=-1, Mixed=0, XTC, HDF5 };

  // Function which tries to guess input data type from file name extensions
  template <typename Iter>
  FileType guessType(Iter begin, Iter end) {

    FileType type = Unknown;

    for ( ; begin != end; ++ begin) {

      std::string ext = fs::path(*begin).extension().string();
      FileType ftype = Unknown;
      if (ext == ".h5") {
        ftype = HDF5;
      } else if (ext == ".xtc") {
        ftype = XTC;
      }

      if (ftype == Unknown) return ftype;
      if (type == Unknown) {
        type = ftype;
      } else if (type == XTC or type == HDF5) {
        if (ftype != type) return Mixed;
      }
    }

    return type;
  }

}

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
  , m_files( "data-file",   "file name(s) with input data", std::list<std::string>() )
  , m_modules()
  , m_state()
{
  addOption( m_calibDirOpt ) ;
  addOption( m_configOpt ) ;
  addOption( m_expNameOpt ) ;
  addOption( m_jobNameOpt ) ;
  addOption( m_modulesOpt ) ;
  addOption( m_maxEventsOpt ) ;
  addOption( m_skipEventsOpt ) ;
  addArgument( m_files ) ;

  m_newStateMethods[None] = 0;
  m_newStateMethods[Configured] = &Module::beginJob;
  m_newStateMethods[Running] = &Module::beginRun;
  m_newStateMethods[Scanning] = &Module::beginCalibCycle;

  m_closeStateMethods[None] = 0;
  m_closeStateMethods[Configured] = &Module::endJob;
  m_closeStateMethods[Running] = &Module::endRun;
  m_closeStateMethods[Scanning] = &Module::endCalibCycle;
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
  // if neither -m nor -c specified then try to read psana.cfg
  std::string cfgFile = m_configOpt.value();
  if (cfgFile.empty() and m_modulesOpt.value().empty()) {
    cfgFile = "psana.cfg";
  }

  // start with reading configuration file
  std::auto_ptr<ConfigSvc::ConfigSvcImplI> cfgImpl;
  if (cfgFile.empty()) {
    cfgImpl.reset( new ConfigSvc::ConfigSvcImplFile() );
  } else {
    cfgImpl.reset( new ConfigSvc::ConfigSvcImplFile(cfgFile) );
  }

  // initialize config service
  ConfigSvc::ConfigSvc::init(cfgImpl);
  ConfigSvc::ConfigSvc cfgsvc;

  // command-line -m options override config file values
  if (not m_modulesOpt.value().empty()) {
    std::string modlist;
    const std::list<std::string>& modules = m_modulesOpt.value();
    for(std::list<std::string>::const_iterator it = modules.begin(); it != modules.end(); ++ it) {
      if (not modlist.empty()) modlist += ' ';
      modlist += *it;
    }
    MsgLogRoot(trace, "set module list to '" << modlist << "'");
    cfgsvc.put("psana", "modules", modlist);
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
      cfgsvc.put("psana", "instrument", instrName);
      cfgsvc.put("psana", "experiment", expName);
  }

  // set event numbers
  if (m_maxEventsOpt.value()) {
      cfgsvc.put("psana", "events", boost::lexical_cast<std::string>(m_maxEventsOpt.value()));
  }
  if (m_skipEventsOpt.value()) {
      cfgsvc.put("psana", "skip-events", boost::lexical_cast<std::string>(m_skipEventsOpt.value()));
  }

  // set calib dir name if specified
  if (not m_calibDirOpt.value().empty()) {
    cfgsvc.put("psana", "calib-dir", m_calibDirOpt.value());
  }

  // get list of modules to load
  std::list<std::string> moduleNames = cfgsvc.getList("psana", "modules");

  // list of files could come from config file and overriden by command line
  std::list<std::string> files(m_files.begin(), m_files.end());
  if (files.empty()) {
    files = cfgsvc.getList("psana", "files", std::list<std::string>());
  }
  if (files.empty()) {
    MsgLogRoot(error, "no input data specified");
    return 2;
  }

  DynLoader loader;

  // Load input module, by default use XTC input even if cannot correctly
  // guess types of the input files
  std::string iname = "PSXtcInput.XtcInputModule";
  FileType ftype = ::guessType(files.begin(), files.end());
  if (ftype == Mixed) {
    MsgLogRoot(error, "Mixed input file types");
    return -1;
  } else if (ftype == HDF5) {
    iname = "PSHdf5Input.Hdf5InputModule";
  }
  boost::shared_ptr<psana::InputModule> input(loader.loadInputModule(iname));
  MsgLogRoot(trace, "Loaded input module " << iname);

  // pass file names to the configuration so that input module can find them
  typedef AppUtils::AppCmdArgList<std::string>::const_iterator FileIter;
  std::string flist;
  for (FileIter it = files.begin(); it != files.end(); ++it ) {
    if (not flist.empty()) flist += " ";
    flist += *it;
  }
  cfgsvc.put(iname, "files", flist);

  // instantiate all user modules
  for ( std::list<std::string>::const_iterator it = moduleNames.begin(); it != moduleNames.end() ; ++ it ) {
    m_modules.push_back(loader.loadModule(*it));
    MsgLogRoot(trace, "Loaded module " << m_modules.back()->name());
  }

  // get/build job name
  std::string jobName = m_jobNameOpt.value();
  if (jobName.empty() and not files.empty()) {
    boost::filesystem::path path = files.front();
    jobName = path.stem().string();
  }
  MsgLogRoot(debug, "job name = " << jobName);

  // get calib directory name
  std::string calibDir = cfgsvc.getStr("psana", "calib-dir", "/reg/d/psdm/{instr}/{exp}/calib");

  // instantiate experiment name provider
  boost::shared_ptr<PSEnv::IExpNameProvider> expNameProvider;
  if(not cfgsvc.getStr("psana", "experiment", "").empty()) {
    const std::string& instr = cfgsvc.getStr("psana", "instrument", "");
    const std::string& exp = cfgsvc.getStr("psana", "experiment", "");
    expNameProvider = boost::make_shared<ExpNameFromConfig>(instr, exp);
  } else if (ftype == XTC) {
    expNameProvider = boost::make_shared<ExpNameFromXtc>(files);
  } else {
    expNameProvider = boost::make_shared<ExpNameFromConfig>("", "");
  }

  // Setup environment
  PSEnv::Env env(jobName, expNameProvider, calibDir);
  MsgLogRoot(debug, "instrument = " << env.instrument() << " experiment = " << env.experiment());
  MsgLogRoot(debug, "calibDir = " << env.calibDir());

  // Start with beginJob for everyone
  {
    Event evt(boost::make_shared<PSEvt::ProxyDict>());
    input->beginJob(evt, env);
    Module::Status stat = callModuleMethod(&Module::beginJob, evt, env, true);
    if (stat != Module::OK) return 1;
    m_state.push(Configured);
  }

  // event loop
  bool stop = false ;
  while (not stop) {

    // Create event object
    Event evt(boost::make_shared<PSEvt::ProxyDict>());

    // run input module to populate event
    InputModule::Status istat = input->event(evt, env);
    MsgLogRoot(debug, "input.event() returned " << istat);

    // check input status
    if (istat == InputModule::Skip) continue;
    if (istat == InputModule::Stop) break;
    if (istat == InputModule::Abort) {
      MsgLogRoot(info, "Input module requested abort");
      return 1;
    }

    // dispatch event to particular method based on event type
    if (istat == InputModule::DoEvent) {

      Module::Status stat = callModuleMethod(&Module::event, evt, env, false);
      if (stat == Module::Abort) return 1;
      if (stat == Module::Stop) break;

    } else {

      State unwindTo = None;
      State newState = None;
      if (istat == InputModule::BeginRun) {
        unwindTo = Configured;
        newState = Running;
      } else if (istat == InputModule::BeginCalibCycle) {
        unwindTo = Running;
        newState = Scanning;
      } else if (istat == InputModule::EndCalibCycle) {
        unwindTo = Running;
      } else if (istat == InputModule::EndRun) {
        unwindTo = Configured;
      }

      Module::Status stat = unwind(unwindTo, evt, env);
      if (stat == Module::Abort) return 1;
      if (stat == Module::Stop) break;
      if (newState != None) {
        stat = this->newState(newState, evt, env);
        if (stat == Module::Abort) return 1;
        if (stat == Module::Stop) break;
      }

    }
  }

  // close all transitions
  {
    Event evt(boost::make_shared<PSEvt::ProxyDict>());
    input->endJob(evt, env);
    unwind(None, evt, env, true);
  }

  // cleanup
  m_modules.clear();

  // return 0 on success, other values for error (like main())
  return 0 ;
}


Module::Status
PSAnaApp::newState(State state, Event& evt, Env& env)
{
  m_state.push(state);
  return callModuleMethod(m_newStateMethods[state], evt, env, true);
}


Module::Status
PSAnaApp::closeState(Event& evt, Env& env)
{
  State state = m_state.top();
  m_state.pop();
  return callModuleMethod(m_closeStateMethods[state], evt, env, true);
}


Module::Status
PSAnaApp::unwind(State newState, Event& evt, Env& env, bool ignoreStatus)
{
  while (not m_state.empty() and m_state.top() > newState) {
    Module::Status stat = closeState(evt, env);
    if (not ignoreStatus and stat != Module::OK) return stat;
  }
  return Module::OK;
}

//
// Call given method for all defined modules, ignoreSkip should be set
// to false for event() method, true for everything else
//
Module::Status
PSAnaApp::callModuleMethod(ModuleMethod method, Event& evt, Env& env, bool ignoreSkip)
{
  Module::Status stat = Module::OK;

  if (ignoreSkip) {

    // call all modules, do not skip any one of them

    for (std::vector<boost::shared_ptr<Module> >::const_iterator it = m_modules.begin() ; it != m_modules.end() ; ++it) {
      boost::shared_ptr<Module> mod = *it;

      // clear module status
      mod->reset();

      // call the method
      ((*mod).*method)(evt, env);

      // check what module wants to tell us
      if (mod->status() == Module::Skip) {
        // silently ignore Skip
      } else if (mod->status() == Module::Stop) {
        // set the flag but continue
        MsgLogRoot(info, "module " << mod->name() << " requested stop");
        stat = Module::Stop;
      } else if (mod->status() == Module::Abort) {
        // abort immediately
        MsgLogRoot(info, "module " << mod->name() << " requested abort");
        stat = Module::Abort;
        break;
      }
    }

  } else {

    // call all modules, respect Skip flag

    for (std::vector<boost::shared_ptr<Module> >::const_iterator it = m_modules.begin() ; it != m_modules.end() ; ++it) {
      boost::shared_ptr<Module> mod = *it;

      // clear module status
      mod->reset();

      // call the method, skip regular modules if skip status is set, but
      // still call special modules which are interested in all events
      if (stat == Module::OK or mod->observeAllEvents()) {
        ((*mod).*method)(evt, env);
      }

      // check what module wants to tell us
      if (mod->status() == Module::Skip) {

        // Set the skip flag but continue as there may be modules interested in every event
        MsgLogRoot(trace, "module " << mod->name() << " requested skip");
        if (stat == Module::OK) stat = Module::Skip;

        // add special flag to event
        if (not evt.exists<int>("__psana_skip_event__")) {
          evt.put(boost::make_shared<int>(1), "__psana_skip_event__");
        }

      } else if (mod->status() == Module::Stop) {
        // stop right here
        MsgLogRoot(info, "module " << mod->name() << " requested stop");
        stat = Module::Stop;
        break;
      } else if (mod->status() == Module::Abort) {
        // abort immediately
        MsgLogRoot(info, "module " << mod->name() << " requested abort");
        stat = Module::Abort;
        break;
      }
    }

  }

  return stat;
}

} // namespace psana
