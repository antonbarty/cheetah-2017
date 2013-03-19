//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Configurable...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana/Configurable.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana {

//----------------
// Constructors --
//----------------
Configurable::Configurable (const std::string& name)
  : m_name(name)
  , m_className(name)
  , m_context(Context::get())
{
  // get class name from module name
  std::string::size_type p = m_className.find(':');
  if (p != std::string::npos) {
    m_className.erase(p);
  }
}

//--------------
// Destructor --
//--------------
Configurable::~Configurable ()
{
}

} // namespace psana
