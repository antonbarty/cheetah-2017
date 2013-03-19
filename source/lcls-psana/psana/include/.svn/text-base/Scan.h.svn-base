#ifndef PSANA_SCAN_H
#define PSANA_SCAN_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Scan.
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
#include "PSEnv/Env.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------
namespace psana {
class ScanIter;
}

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana {

/// @addtogroup psana

/**
 *  @ingroup psana
 *
 *  @brief Class representing a scan (calib cycle).
 *  
 *  Main purpose of this class is to provide iteration over contained 
 *  events.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class Scan  {
public:

  /// Default constructor makes "null" scan object
  Scan();

  // Constructor takes event loop object
  Scan(const boost::shared_ptr<EventLoop>& evtLoop);
  
  // Destructor
  ~Scan () ;
  
  /// This object is converted to true for non-null instance
  operator bool() const { return bool(m_evtLoop); }
  bool operator!() const { return not bool(m_evtLoop); }

  /// Get environment object, cannot be called for "null" object
  PSEnv::Env& env() const;

  /// Returns iterator for events in this scan
  EventIter events() { return EventIter(m_evtLoop, EventLoop::EndCalibCycle); }

protected:

private:

  // Data members
  boost::shared_ptr<EventLoop> m_evtLoop;

};

} // namespace psana

#endif // PSANA_SCAN_H
