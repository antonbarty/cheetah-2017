#include "pdsdata/opal1k/ConfigV1.hh"
#include "pdsdata/camera/FrameCoord.hh"
#include "pdsdata/xtc/DetInfo.hh"

#include <string.h>

using namespace Pds;
using namespace Opal1k;

enum { Black_Offset =  0,
       Gain_Offset  = 16 };

enum { Depth_Offset              = 0,
       Binning_Offset            = 4,
       Mirroring_Offset          = 8,
       Vertical_Remap_Offset     =12,
       Output_LUT_Enable_Offset  =13,
       Pixel_Corr_Enable_Offset  =14 };

static const int Output_LUT_Size = 4096;

ConfigV1::ConfigV1() {}

ConfigV1::ConfigV1(unsigned  short black,
		   unsigned  short gain,
		   Depth     depth,
		   Binning   binning,
		   Mirroring mirroring,
		   bool      vertical_remap,
		   bool      enable_pixel_correction ) :
  _offsetAndGain    ( ((black&0xffff)<< Black_Offset) |
		      ((gain &0xffff)<< Gain_Offset ) ),
  _outputOptions    ( ( depth                   << Depth_Offset) |
		      ( binning                 << Binning_Offset) |
		      ( mirroring               << Mirroring_Offset) | 
		      ( vertical_remap          ? 1 << Vertical_Remap_Offset : 0) |
		      ( enable_pixel_correction ? 1 << Pixel_Corr_Enable_Offset : 0) ),
  _defectPixelCount ( 0 )
{
}

ConfigV1::ConfigV1(unsigned  short black,
		   unsigned  short gain,
		   Depth     depth,
		   Binning   binning,
		   Mirroring mirroring,
		   bool      vertical_remap,
		   bool      enable_pixel_correction,
		   const uint16_t* output_lut) :
  _offsetAndGain( ((black&0xffff)<< Black_Offset) |
		  ((gain &0xffff)<< Gain_Offset ) ),
  _outputOptions( ( depth                   << Depth_Offset) |
		  ( binning                 << Binning_Offset) |
		  ( mirroring               << Mirroring_Offset) | 
		  ( vertical_remap          ? 1 << Vertical_Remap_Offset : 0) |
		  ( enable_pixel_correction ? 1 << Pixel_Corr_Enable_Offset : 0) |
		  (                           1 << Output_LUT_Enable_Offset) ),
  _defectPixelCount ( 0 )
{
  memcpy( this+1, output_lut, Output_LUT_Size*sizeof(uint16_t));
}

unsigned short  ConfigV1::black_level() const
{ return (_offsetAndGain >> Black_Offset)&0xffff; }

unsigned short  ConfigV1::gain_percent() const
{ return (_offsetAndGain >> Gain_Offset)&0xffff; }

unsigned short  ConfigV1::output_offset() const
{ 
  unsigned offset(black_level());
  offset *= gain_percent();
  offset /= 100;
  return offset;
}

ConfigV1::Depth           ConfigV1::output_resolution() const
{ return Depth((_outputOptions >> Depth_Offset)&0xf); }

unsigned        ConfigV1::output_resolution_bits() const
{ return 8 + (((_outputOptions >> Depth_Offset)&3)<<1); }

ConfigV1::Binning         ConfigV1::vertical_binning() const
{ return Binning((_outputOptions >> Binning_Offset)&0xf); }

ConfigV1::Mirroring       ConfigV1::output_mirroring() const
{ return Mirroring((_outputOptions >> Mirroring_Offset)&0xf); }

bool            ConfigV1::vertical_remapping() const
{ return _outputOptions & (1<<Vertical_Remap_Offset); }

bool            ConfigV1::defect_pixel_correction_enabled() const
{ return _outputOptions & (1<<Pixel_Corr_Enable_Offset); }

bool            ConfigV1::output_lookup_table_enabled() const
{ return _outputOptions & (1<<Output_LUT_Enable_Offset); }

const 
uint16_t* ConfigV1::output_lookup_table() const
{ return reinterpret_cast<const uint16_t*>(this+1); }

unsigned        ConfigV1::number_of_defect_pixels() const
{ return _defectPixelCount; }

void            ConfigV1::set_number_of_defect_pixels(unsigned count) 
{ _defectPixelCount = count; }

const
Camera::FrameCoord*     ConfigV1::defect_pixel_coordinates() const
{
  return output_lookup_table_enabled() ? 
    reinterpret_cast<const Camera::FrameCoord*>(output_lookup_table()+Output_LUT_Size) :
    reinterpret_cast<const Camera::FrameCoord*>(this+1);
}

Camera::FrameCoord*     ConfigV1::defect_pixel_coordinates()
{
  return output_lookup_table_enabled() ?
    reinterpret_cast<Camera::FrameCoord*>
    (const_cast<uint16_t*>(output_lookup_table()+Output_LUT_Size)) :
    reinterpret_cast<Camera::FrameCoord*>(this+1);
}

unsigned        ConfigV1::size() const
{ 
  return sizeof(*this) + 
    number_of_defect_pixels()*sizeof(Camera::FrameCoord) +
    (output_lookup_table_enabled() ? Output_LUT_Size*sizeof(uint16_t) : 0);
}

unsigned ConfigV1::max_row_pixels   (const DetInfo& info)
{
  switch(info.device()) {
  case Pds::DetInfo::Opal1000: return 1024;
  case Pds::DetInfo::Opal1600: return 1200;
  case Pds::DetInfo::Opal2000: return 1080;
  case Pds::DetInfo::Opal4000: return 1752;
  case Pds::DetInfo::Opal8000: return 2472;
  default:       return 0;
  }
}

unsigned ConfigV1::max_column_pixels(const DetInfo& info)
{
  switch(info.device()) {
  case Pds::DetInfo::Opal1000: return 1024;
  case Pds::DetInfo::Opal1600: return 1600;
  case Pds::DetInfo::Opal2000: return 1920;
  case Pds::DetInfo::Opal4000: return 2336;
  case Pds::DetInfo::Opal8000: return 3296;
  default:       return 0;
  }
}

