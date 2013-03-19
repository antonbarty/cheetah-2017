#ifndef PSANA_RUN_H
#define PSANA_RUN_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Run.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <boost/shared_ptr.hpp>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psana/EventIter.h"
#include "psana/EventLoop.h"
#include "psana/ScanIter.h"
#include "PSEnv/Env.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------
namespace psana {
class RunIter;
}

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana {

/// @addtogroup psana

/**
 *  @ingroup psana
 *
 *  @brief Class representing a run.
 *  
 *  Main purpose of this class is to provide iteration over contained 
 *  events or scans.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class Run  {
public:

  /// Default constructor makes "null" run object
  Run();

  // Constructor takes event loop object
  Run(const boost::shared_ptr<EventLoop>& evtLoop, int run);
  
  // Destructor
  ~Run () ;
  
  /// This object is converted to true for non-null instance
  operator bool() const { return bool(m_evtLoop); }
  bool operator!() const { return not bool(m_evtLoop); }

  /// Get environment object, cannot be called for "null" object
  PSEnv::Env& env() const;

  /// Get run number, -1 returned for if unknown
  int run() const { return m_run; }
  
  /// Returns iterator for events in this run
  EventIter events() { return EventIter(m_evtLoop, EventLoop::EndRun); }

  /// Returns iterator for scans in this run
  ScanIter scans() { return ScanIter(m_evtLoop, EventLoop::EndRun); }


protected:

private:

  // Data members
  boost::shared_ptr<EventLoop> m_evtLoop;
  int m_run;

};

} // namespace psana

#endif // PSANA_RUN_H
