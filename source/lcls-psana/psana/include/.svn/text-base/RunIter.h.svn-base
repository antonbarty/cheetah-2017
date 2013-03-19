#ifndef PSANA_RUNITER_H
#define PSANA_RUNITER_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class RunIter.
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
#include "psana/Run.h"

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
 *  @brief Class representing iterator over runs.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class RunIter  {
public:

  typedef Run value_type;
    
  /// Default constructor makes invalid iterator
  RunIter () ;

  /// Constructor takes event loop instance.
  RunIter (const boost::shared_ptr<EventLoop>& evtLoop);

  // Destructor
  ~RunIter () ;

  /// get next scan, when done returns object which is convertible to "false"
  value_type next();

protected:

private:

  // Data members
  boost::shared_ptr<EventLoop> m_evtLoop;

};

} // namespace psana

#endif // PSANA_RUNITER_H
