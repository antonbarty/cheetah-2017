#ifndef PSANA_INPUTMODULE_H
#define PSANA_INPUTMODULE_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: InputModule.h 3038 2012-03-08 22:12:17Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class InputModule.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <string>
#include <iosfwd>
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

// define macro for definition of the factory function
#if defined(PSANACAT2_)
#undef PSANACAT2_
#endif
#define PSANACAT2_(a,b) a ## b
#define PSANA_INPUT_MODULE_FACTORY(UserModule) \
  extern "C" \
  psana::InputModule* \
  PSANACAT2_(_psana_input_module_,UserModule)(const std::string& name) {\
    return new UserModule(name);\
  }

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana {

/**
 *  @ingroup psana
 *  
 *  @brief Base class for psana input modules.
 *  
 *  Psana input module is responsible for reading input files or other event 
 *  sources and decide what framework should do with the next read event.
 *  In a sense input module drives the framework event loop based on the input 
 *  event data.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @see Module
 *
 *  @version \$Id: InputModule.h 3038 2012-03-08 22:12:17Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andrei Salnikov
 */

class InputModule : boost::noncopyable, protected Configurable {
public:

  /**
   *  @brief Event processing status.
   *  
   *  The value returned from event() signals to framework what it should do next.
   */
  enum Status { 
    BeginRun,        ///< beginRun() should be called for all modules
    BeginCalibCycle, ///< beginCalibCycle() should be called for all modules
    DoEvent,         ///< event() should be called for all modules
    EndCalibCycle,   ///< endCalibCycle() should be called for all modules
    EndRun,          ///< endRun() should be called for all modules
    Skip,            ///< skip all remaining modules for this event
    Stop,            ///< gracefully finish with the event loop
    Abort            ///< abort immediately, no finalization
  };

  // Destructor
  virtual ~InputModule () ;

  // Returns the name of the module
  using Configurable::name;
  
  // Returns the class name of the module
  using Configurable::className;
  
  /**
   *  @brief Method which is called once at the beginning of the job.
   *  
   *  Input module is supposed to populate environment with the configuration
   *  objects and EPICS data from Configure transition.
   *  
   *  @param[out] evt    Event object
   *  @param[out] env    Environment object
   */ 
  virtual void beginJob(Event& evt, Env& env);
  
  /**
   *  @brief Method which is called for the next event in the event loop.
   *
   *  Input module should try to read next event from input source and fill 
   *  event object and environment object if possible. Depending on what has 
   *  been read it also signals framework what to do next by returning one of 
   *  the Status enum values.
   *    
   *  @param[out] evt    Event object
   *  @param[out] env    Environment object
   */
  virtual Status event(Event& evt, Env& env) = 0;
  
  /**
   *  @brief Method which is called once at the end of the job
   *  
   *  Input module can stop reading data and close/reset its sources. It does
   *  not need to update environment but can use some data from it.
   *  
   *  @param[out] evt    Event object
   *  @param[out] env    Environment object
   */
  virtual void endJob(Event& evt, Env& env);
  
protected:

  /// Constructor may be called from subclass only.
  InputModule (const std::string& name) ;

private:

};

/// formatting for InputModule::Status enum
std::ostream&
operator<<(std::ostream& out, InputModule::Status stat);

} // namespace psana

#endif // PSANA_INPUTMODULE_H
