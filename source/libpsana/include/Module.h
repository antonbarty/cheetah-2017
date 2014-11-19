#ifndef PSANA_MODULE_H
#define PSANA_MODULE_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: Module.h 6430 2013-06-24 17:12:58Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class Module.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <boost/utility.hpp>

//----------------------
// Base Class Headers --
//----------------------
#include "psana/Configurable.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "PSEnv/Env.h"
#include "PSEvt/Event.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

// this is not nice thing to do but we want to simplify user's life
// and provide bunch of simple interfaces to our system

namespace psana {}
using namespace psana;
using namespace PSEnv;
using namespace PSEvt;
using RootHistoManager::AxisDef;
using PSHist::Axis;
using boost::shared_ptr;

typedef boost::shared_ptr<PSEnv::Env> EnvPtr;
typedef boost::shared_ptr<PSEvt::Event> EventPtr;

// define macro for definition of the factory function
#if defined(PSANACAT2_)
#undef PSANACAT2_
#endif
#define PSANACAT2_(a,b) a ## b
#define PSANA_MODULE_FACTORY(UserModule) \
  extern "C" \
  psana::Module* \
  PSANACAT2_(_psana_module_,UserModule)(const std::string& name) {\
    return new UserModule(name);\
  }

//		---------------------
// 		-- Class Interface --
//		---------------------


namespace psana {

/**
 *  @ingroup psana
 *  
 *  @brief Base class for user modules in psana framework.
 *  
 *  This is the major customization point available to users. All analysis
 *  code should inherit from this class and provide implementation of 
 *  one or few methods which have access to all event and non-event data
 *  being processed by framework. 
 *  
 *  User modules have some influence on the framework event loop, by calling 
 *  one of the skip(), stop(), or terminate() methods user module can signal the
 *  framework to either skip the processing of the current event, stop
 *  analysis gracefully (after closing all output files) or abort anaylis
 *  immediately.
 *  
 *  Subclasses must implement at least event() method, other methods have 
 *  default implementation which does nothing useful.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version \$Id: Module.h 6430 2013-06-24 17:12:58Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andrei Salnikov
 */

class Module : boost::noncopyable, protected Configurable {
public:

  /**
   *  @brief Event processing status.
   *  
   *  The value returned from status() signals to framework what it should do next.
   */
  enum Status { OK,     ///< processing finished normally
                Skip,   ///< skip all remaining modules for this event
                Stop,   ///< finish with the events
                Abort   ///< abort immediately, no finalization
  };
  
  // Destructor
  virtual ~Module () ;

  /// get the name of the module
  using Configurable::name;
  
  /// get the class name of the module
  using Configurable::className;
  
  /**
   *  @brief Method which is called once at the beginning of the job
   *  
   *  @param[in,out] evt  Event object. In this call it does not have any event 
   *                      data but can be used to pass information between modules.
   *  @param[in] env  Environment object. 
   */
  virtual void beginJob(EventPtr evt, EnvPtr env) { beginJob(*evt, *env); }

  virtual void beginJob(Event& evt, Env& env);
  
  /**
   *  @brief Method which is called at the beginning of new run
   *  
   *  @param[in,out] evt  Event object. In this call it does not have any event 
   *                      data but can be used to pass information between modules.
   *  @param[in] env  Environment object. 
   */
  virtual void beginRun(EventPtr evt, EnvPtr env) { beginRun(*evt, *env); }

  virtual void beginRun(Event& evt, Env& env);
  
  /**
   *  @brief Method which is called at the beginning of new calibration cycle (step)
   *  
   *  @param[in,out] evt  Event object. In this call it does not have any event 
   *                      data but can be used to pass information between modules.
   *  @param[in] env  Environment object. 
   */
  virtual void beginCalibCycle(EventPtr evt, EnvPtr env) { beginCalibCycle(*evt, *env); }
  
  virtual void beginCalibCycle(Event& evt, Env& env);
  
  /**
   *  @brief Method which is called with event data
   *  
   *  @param[in,out] evt  Event object.
   *  @param[in] env  Environment object. 
   */
  virtual void event(EventPtr evt, EnvPtr env) { event(*evt, *env); }

  virtual void event(Event& evt, Env& env) {}
  
  /**
   *  @brief Method which is called at the end of the calibration cycle (step)
   *  
   *  @param[in,out] evt  Event object. In this call it does not have any event 
   *                      data but can be used to pass information between modules.
   *  @param[in] env  Environment object. 
   */
  virtual void endCalibCycle(EventPtr evt, EnvPtr env) { endCalibCycle(*evt, *env); }

  virtual void endCalibCycle(Event& evt, Env& env);

  /**
   *  @brief Method which is called at the end of the run
   *  
   *  @param[in,out] evt  Event object. In this call it does not have any event 
   *                      data but can be used to pass information between modules.
   *  @param[in] env  Environment object. 
   */
  virtual void endRun(EventPtr evt, EnvPtr env) { endRun(*evt, *env); }

  virtual void endRun(Event& evt, Env& env);

  /**
   *  @brief Method which is called once at the end of the job
   *  
   *  @param[in,out] evt  Event object. In this call it does not have any event 
   *                      data but can be used to pass information between modules.
   *  @param[in] env  Environment object. 
   */
  virtual void endJob(EventPtr evt, EnvPtr env) { endJob(*evt, *env); }

  virtual void endJob(Event& evt, Env& env);

  /// reset module status
  void reset() { m_status = OK; }
  
  /// get status
  Status status() const { return m_status; }

  /// Returns true if this module is interested in all events including skipped
  bool observeAllEvents() const { return m_observeAllEvents; }
  
protected:

  /// The one and only constructor, needs module name.
  Module (const std::string& name, bool observeAllEvents = false) ;

  /**
   *   @brief Signal framework to skip current event and do not call other downstream modules.
   *
   *   Note that this method does not skip code in the current module, control is
   *   returned back to the module. If you want to stop processing after this call
   *   then add a return statement.
   */
  void skip() { m_status = Skip; }
  
  /**
   *   @brief Signal framework to stop event loop and finish job gracefully.
   *
   *   Note that this method does not terminate processing in the current module.
   *   If you want to stop processing after this call then add a return statement.
   */
  void stop() { m_status = Stop; }
  
  /**
   *  @brief Signal framework to terminate immediately.
   *
   *   Note that this method does not terminate processing in the current module.
   *   If you want to stop processing after this call then add a return statement.
   */
  void terminate() { m_status = Abort; }

private:

  Status m_status;  ///< Current event processing status
  bool m_observeAllEvents; ///< If true then this module will receive all events, event skipped ones

};

// formatting for enum
std::ostream&
operator<<(std::ostream& out, Module::Status stat);

} // namespace psana

#endif // PSANA_MODULE_H
