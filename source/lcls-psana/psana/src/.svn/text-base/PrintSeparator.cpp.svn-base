//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class PrintSeparator...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/PrintSeparator.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana;
PSANA_MODULE_FACTORY(PrintSeparator)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
PrintSeparator::PrintSeparator (const std::string& name)
  : Module(name)
{
  std::string sep = configStr("separator", "=");
  unsigned repeat = config("repeat", 80U);
  m_separator.reserve(sep.size()*repeat);
  for (unsigned i = 0; i < repeat; ++ i) {
    m_separator += sep;
  }
}

//--------------
// Destructor --
//--------------
PrintSeparator::~PrintSeparator ()
{
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
PrintSeparator::event(Event& evt, Env& env)
{
  MsgLogRoot(info, m_separator);    
}
  
} // namespace psana
