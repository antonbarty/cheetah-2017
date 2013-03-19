#ifndef PSANA_SCANITER_H
#define PSANA_SCANITER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ScanIter.
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
#include "psana/Scan.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------
namespace psana {
class EventLoop;
}

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana {

/// @addtogroup psana

/**
 *  @ingroup psana
 *
 *  @brief Class representing iterator over scans (calib cycles)
 *
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class ScanIter  {
public:

  typedef Scan value_type;
    
  /// Default constructor makes invalid iterator
  ScanIter () ;

  /**
   *  @brief Constructor takes event loop instance and "stop event type".
   *
   *  Do not use EventLoop::Event for stop type, first it does not make
   *  sense, second this iterator uses it for special purpose.
   */
  ScanIter (const boost::shared_ptr<EventLoop>& evtLoop, EventLoop::EventType stopType);

  // Destructor
  ~ScanIter () ;

  /// get next scan, when done returns object which is convertible to "false"
  value_type next();

protected:

private:

  // Data members
  boost::shared_ptr<EventLoop> m_evtLoop;
  EventLoop::EventType m_stopType;

};

} // namespace psana

#endif // PSANA_SCANITER_H
