//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class PSAna...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/PSAna.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "ConfigSvc/ConfigSvc.h"
#include "ConfigSvc/ConfigSvcImplFile.h"
#include "IData/Dataset.h"
#include "MsgLogger/MsgLogger.h"
#include "psana/DynLoader.h"
#include "psana/ExpNameFromConfig.h"
#include "psana/ExpNameFromDs.h"
#include "PSEnv/Env.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace fs = boost::filesystem ;

namespace {

  const char* logger = "PSAna";

  enum FileType { Unknown=-1, Mixed=0, XTC, HDF5 };

  // Function which tries to guess input data type from file name extensions
  template <typename Iter>
  FileType guessType(Iter begin, Iter end) {

    FileType type = Unknown;

    for ( ; begin != end; ++ begin) {

      IData::Dataset ds(*begin);

      FileType ftype = Unknown;
      if (ds.exists("h5")) {
        ftype = HDF5;
      } else if (ds.exists("xtc")) {
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

//----------------
// Constructors --
//----------------
PSAna::PSAna(const std::string& config, const std::map<std::string, std::string>& options)
  : m_context(Context::generate())
  , m_modules()
{
  Context::set(m_context);

  // initialize configuration service, this can only be done once
  boost::shared_ptr<ConfigSvc::ConfigSvcImplI> cfgImpl = boost::make_shared<ConfigSvc::ConfigSvcImplFile>(config);
  ConfigSvc::ConfigSvc::init(cfgImpl, m_context);

  // for backward compaibility also initialize config service in global context
  if (not ConfigSvc::ConfigSvc::initialized()) {
    ConfigSvc::ConfigSvc::init(boost::make_shared<ConfigSvc::ConfigSvcImplFile>());
  }

  ConfigSvc::ConfigSvc cfgsvc(m_context);
  ConfigSvc::ConfigSvc glbcfgsvc;

  // copy all options
  for (std::map<std::string, std::string>::const_iterator it = options.begin(); it != options.end(); ++ it) {
    std::string section;
    std::string option = it->first;
    std::string::size_type p = option.rfind('.');
    if (p == std::string::npos) {
      section = "psana";
    } else {
      section = std::string(option, 0, p);
      option.erase(0, p+1);
    }
    cfgsvc.put(section, option, it->second);
    // and update global config as well
    glbcfgsvc.put(section, option, it->second);
  }

  // get list of modules to load
  std::vector<std::string> moduleNames = cfgsvc.getList("psana", "modules", std::vector<std::string>());

  // instantiate all user modules
  DynLoader loader;
  for ( std::vector<std::string>::const_iterator it = moduleNames.begin(); it != moduleNames.end() ; ++ it ) {
    m_modules.push_back(loader.loadModule(*it));
    MsgLog(logger, trace, "Loaded module " << m_modules.back()->name());
  }


}

//--------------
// Destructor --
//--------------
PSAna::~PSAna ()
{
}

/**
 *  @brief Get the list of modules.
 */
std::vector<std::string>
PSAna::modules()
{
  ConfigSvc::ConfigSvc cfgsvc(m_context);
  std::vector<std::string> moduleNames = cfgsvc.getList("psana", "modules", std::vector<std::string>());
  return moduleNames;
}


// Create data source instance for the set of input files/datasets.
DataSource
PSAna::dataSource(const std::vector<std::string>& input)
{
  Context::set(m_context);

  ConfigSvc::ConfigSvc cfgsvc(m_context);

  DataSource dataSrc;

  // if input is empty try to use input from config file
  std::vector<std::string> inputList(input);
  if (inputList.empty()) {
    inputList = cfgsvc.getList("psana", "files", std::vector<std::string>());
  }
  if (inputList.empty()) {
    MsgLog(logger, error, "no input data specified");
    return dataSrc;
  }

  // Guess input data type, by default use XTC input even if cannot correctly
  // guess types of the input files
  std::string iname = "PSXtcInput.XtcInputModule";
  ::FileType ftype = ::guessType(inputList.begin(), inputList.end());
  if (ftype == Mixed) {
    MsgLog(logger, error, "Mixed input file types");
    return dataSrc;
  } else if (ftype == HDF5) {
    iname = "PSHdf5Input.Hdf5InputModule";
  }

  // pass file names to the configuration so that input module can find them
  std::string flist = boost::join(inputList, " ");
  cfgsvc.put(iname, "files", flist);

  // Load input module
  DynLoader loader;
  boost::shared_ptr<psana::InputModule> inputModule(loader.loadInputModule(iname));
  MsgLog(logger, trace, "Loaded input module " << iname);

  // get calib directory name
  std::string calibDir = cfgsvc.getStr("psana", "calib-dir", "/reg/d/psdm/{instr}/{exp}/calib");

  // get/build job name
  std::string jobName = cfgsvc.getStr("psana", "job-name", "");
  if (jobName.empty() and not inputList.empty()) {
    boost::filesystem::path path = inputList.front();
    jobName = path.stem().string();
  }
  MsgLog(logger, debug, "job name = " << jobName);

  // instantiate experiment name provider
  boost::shared_ptr<PSEnv::IExpNameProvider> expNameProvider;
  if(not cfgsvc.getStr("psana", "experiment", "").empty()) {
    const std::string& instr = cfgsvc.getStr("psana", "instrument", "");
    const std::string& exp = cfgsvc.getStr("psana", "experiment", "");
    expNameProvider = boost::make_shared<ExpNameFromConfig>(instr, exp);
  } else {
    expNameProvider = boost::make_shared<ExpNameFromDs>(inputList);
  }

  // Setup environment
  boost::shared_ptr<PSEnv::Env> env = boost::make_shared<PSEnv::Env>(jobName, expNameProvider, calibDir);
  MsgLogRoot(debug, "instrument = " << env->instrument() << " experiment = " << env->experiment());
  MsgLogRoot(debug, "calibDir = " << env->calibDir());

  // make new instance
  dataSrc = DataSource(inputModule, m_modules, env);

  return dataSrc;
}

} // namespace psana
