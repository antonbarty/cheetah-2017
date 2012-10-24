//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class EBeamHist...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/EBeamHist.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/bld.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace psana_examples;
PSANA_MODULE_FACTORY(EBeamHist)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
EBeamHist::EBeamHist (const std::string& name)
  : Module(name)
  , m_ebeamHisto(0)
  , m_chargeHisto(0)
{
  m_ebeamSrc = configStr("eBeamSource", "BldInfo(EBeam)");
}

//--------------
// Destructor --
//--------------
EBeamHist::~EBeamHist ()
{
}

// Method which is called once at the beginning of the job
void 
EBeamHist::beginJob(Event& evt, Env& env)
{
  m_ebeamHisto = env.hmgr().hist1i("ebeamHisto", "ebeamL3Energy value", Axis(1000, 13000, 16000));
  m_chargeHisto = env.hmgr().hist1i("echargeHisto", "ebeamCharge value", Axis(250, 0, 0.25));
}

// Method which is called with event data
void 
EBeamHist::event(Event& evt, Env& env)
{
  shared_ptr<Psana::Bld::BldDataEBeamV1> ebeam = evt.get(m_ebeamSrc);
  if (ebeam.get()) {
    m_ebeamHisto->fill(ebeam->ebeamL3Energy());
    m_chargeHisto->fill(ebeam->ebeamCharge());
  }
}

} // namespace psana_examples
