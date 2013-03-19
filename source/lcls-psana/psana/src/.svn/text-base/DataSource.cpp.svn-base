//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DataSource...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/DataSource.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <boost/make_shared.hpp>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psana/EventLoop.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//      ----------------------------------------
//      -- Public Function Member Definitions --
//      ----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
DataSource::DataSource()
  : m_evtLoop()
{
}

DataSource::DataSource (const boost::shared_ptr<InputModule>& inputModule,
    const std::vector<boost::shared_ptr<Module> >& modules,
    const boost::shared_ptr<PSEnv::Env>& env)
  : m_evtLoop(boost::make_shared<EventLoop>(inputModule, modules, env))
{
}

//--------------
// Destructor --
//--------------
DataSource::~DataSource ()
{
}

/// Get environment object, cannot be called for "null" source
PSEnv::Env&
DataSource::env() const
{
  return m_evtLoop->env();
}

} // namespace psana
