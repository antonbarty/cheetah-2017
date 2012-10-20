//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class ExpNameFromConfig...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/ExpNameFromConfig.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <utility>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "ExpNameDb/ExpNameDatabase.h"
#include "MsgLogger/MsgLogger.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace std;

namespace {

  const char* logger = "ExpNameFromConfig";

}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
ExpNameFromConfig::ExpNameFromConfig (const std::string& instr, const std::string& exp)
  : IExpNameProvider()
  , m_instr(instr)
  , m_exp(exp)
  , m_expNum(0)
{
  ExpNameDb::ExpNameDatabase namedb;
  if (m_instr.empty()) {
    const pair<string, unsigned>& res = namedb.getInstrumentAndID(m_exp);
    if (res.first.empty()) {
      MsgLog(logger, warning, "ExpNameFromConfig: failed to find experiment name " << m_exp);
    }
    m_instr = res.first;
    m_expNum = res.second;
  } else {
    m_expNum = namedb.getID(m_instr, m_exp);
    if (m_expNum == 0) {
      MsgLog(logger, warning, "ExpNameFromConfig: failed to find instrument/experiment name " << m_instr << "/" << m_exp);
    }
  }
}

//--------------
// Destructor --
//--------------
ExpNameFromConfig::~ExpNameFromConfig ()
{
}

} // namespace psana
