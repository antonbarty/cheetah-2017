#ifndef PSANA_DATASOURCE_H
#define PSANA_DATASOURCE_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: DataSource.h 3781 2012-06-11 01:01:41Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class DataSource.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <vector>
#include <boost/shared_ptr.hpp>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psana/EventIter.h"
#include "psana/EventLoop.h"
#include "psana/RunIter.h"
#include "psana/ScanIter.h"
#include "PSEnv/Env.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------
namespace psana {
class InputModule;
class Module;
}


//		---------------------
// 		-- Class Interface --
//		---------------------

namespace psana {

/// @addtogroup psana

/**
 *  @ingroup psana
 *
 *  @brief Class representing a data source for psana framework.
 *
 *  Class encapsulates input data in the form of input module and
 *  provides different ways to iterator over those data.
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id: DataSource.h 3781 2012-06-11 01:01:41Z salnikov@SLAC.STANFORD.EDU $
 *
 *  @author Andy Salnikov
 */

class DataSource {
public:

  /**
   *  @brief Default constructor makes "null" data source
   */
  DataSource();

  /**
   *  @brief Make an instance of data source.
   *
   *  Constructor takes instance of input module, and a list of
   *  user modules.
   */
  DataSource(const boost::shared_ptr<InputModule>& inputModule,
             const std::vector<boost::shared_ptr<Module> >& modules,
             const boost::shared_ptr<PSEnv::Env>& env);

  // Destructor
  ~DataSource();

  /**
   *  Returns true if data source has no data ("null" source)
   */
  bool empty() const { return not m_evtLoop; }

  /// Get environment object, cannot be called for "null" source
  PSEnv::Env& env() const;

  /// Returns iterator for events
  EventIter events() { return EventIter(m_evtLoop, EventLoop::None); }

  /// Returns iterator for scans
  ScanIter scans() { return ScanIter(m_evtLoop, EventLoop::None); }

  /// Returns iterator for runs
  RunIter runs() { return RunIter(m_evtLoop); }

protected:

private:

  // Data members
  boost::shared_ptr<EventLoop> m_evtLoop;
};

} // namespace psana

#endif // PSANA_DATASOURCE_H
