//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: RunFileIterLive.cpp 6040 2013-03-29 23:32:41Z salnikov@SLAC.STANFORD.EDU $
//
// Description:
//	Class RunFileIterLive...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "XtcInput/RunFileIterLive.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "XtcInput/StreamFileIterLive.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace XtcInput {

//--------------
// Destructor --
//--------------
RunFileIterLive::~RunFileIterLive ()
{
}

/**
 *  @brief Return stream iterator for next run.
 *
 *  Zero pointer is returned after last run.
 */
boost::shared_ptr<StreamFileIterI>
RunFileIterLive::next()
{
  boost::shared_ptr<StreamFileIterI> next;
  if (not m_runs.empty()) {
    Runs::iterator s = m_runs.begin();
    m_run = *s;
    next = boost::make_shared<StreamFileIterLive>(m_expNum, m_run, m_stream, m_liveTimeout, m_filesdb);
    m_runs.erase(s);
  }

  return next;
}

/**
 *  @brief Return run number for the set of files returned from last next() call.
 */
unsigned
RunFileIterLive::run() const
{
  return m_run;
}

} // namespace XtcInput
