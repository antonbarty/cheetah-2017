//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class DumpQuartz...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "psana_examples/DumpQuartz.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
#include "psddl_psana/quartz.ddl.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

using namespace psana_examples;
PSANA_MODULE_FACTORY(DumpQuartz)

namespace {
  
  void
  printFrameCoord(std::ostream& str, const Psana::Camera::FrameCoord& coord) 
  {
    str << "(" << coord.column() << ", " << coord.row() << ")";
  }
}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace psana_examples {

//----------------
// Constructors --
//----------------
DumpQuartz::DumpQuartz (const std::string& name)
  : Module(name)
{
  m_src = configStr("source", "DetInfo(:Quartz4A150)");
}

//--------------
// Destructor --
//--------------
DumpQuartz::~DumpQuartz ()
{
}

// Method which is called at the beginning of the calibration cycle
void 
DumpQuartz::beginCalibCycle(Event& evt, Env& env)
{
  MsgLog(name(), trace, "in beginCalibCycle()");

  shared_ptr<Psana::Quartz::ConfigV1> config = env.configStore().get(m_src);
  if (config.get()) {
    
    WithMsgLog(name(), info, str) {
      str << "Psana::Quartz::ConfigV1:";
      str << "\n  black_level = " << config->black_level();
      str << "\n  gain_percent = " << config->gain_percent();
      str << "\n  output_resolution = " << config->output_resolution();
      str << "\n  horizontal_binning = " << config->horizontal_binning();
      str << "\n  vertical_binning = " << config->vertical_binning();
      str << "\n  output_mirroring = " << config->output_mirroring();
      str << "\n  output_offset = " << config->output_offset();
      str << "\n  output_resolution_bits = " << config->output_resolution_bits();
      str << "\n  defect_pixel_correction_enabled = " << int(config->defect_pixel_correction_enabled());
      str << "\n  output_lookup_table_enabled = " << int(config->output_lookup_table_enabled());
      
      if (config->output_lookup_table_enabled()) {        
        const ndarray<uint16_t, 1>& output_lookup_table = config->output_lookup_table();
        str << "\n  output_lookup_table =";
        for (unsigned i = 0; i < output_lookup_table.size(); ++ i) {
          str << ' ' << output_lookup_table[i];
        }
        
      }
      
      
      if (config->number_of_defect_pixels()) {
        str << "\n  defect_pixel_coordinates =";
        const ndarray<Psana::Camera::FrameCoord, 1>& coord = config->defect_pixel_coordinates();
        for (unsigned i = 0; i < coord.size(); ++ i) {
          str << " ";
          printFrameCoord(str, coord[i]);
        }
      }
    }
    
  }
}

// Method which is called with event data
void 
DumpQuartz::event(Event& evt, Env& env)
{
  // Quartz device produces standard Camera::Frame type of images,
  // check DumpCamera module which can dump those images
}
  
} // namespace psana_examples
