#include "pdsdata/evr/PulseConfig.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

enum { Trigger_Shift=0, Set_Shift=8, Clear_Shift=16 };
enum { Polarity_Shift=0, 
       Map_Set_Ena_Shift=1,
       Map_Reset_Ena_Shift=2,
       Map_Trigger_Ena_Shift=3 };

PulseConfig::PulseConfig () {}

PulseConfig::PulseConfig (unsigned pulse,
			  int      trigger,       // Pulse input control
			  int      set,
			  int      clear,
			  bool polarity,          // Pulse output control
			  bool map_set_ena,
			  bool map_reset_ena,
			  bool map_trigger_ena,
			  unsigned prescale,      // Pulse parameters
			  unsigned delay,
			  unsigned width) :
  _pulse         (pulse),
  _input_control ( ((trigger+1) << Trigger_Shift) |
		   (    (set+1) << Set_Shift    ) |
		   (  (clear+1) << Clear_Shift  ) ),
  _output_control( (       polarity ? (1<<Polarity_Shift       ) : 0) |
		   (  map_reset_ena ? (1<<Map_Reset_Ena_Shift  ) : 0) |
		   (map_trigger_ena ? (1<<Map_Trigger_Ena_Shift) : 0) ),
  _prescale      (prescale),
  _delay         (delay),
  _width         (width)
{
}

unsigned PulseConfig::pulse() const
{ return _pulse; }

int PulseConfig::trigger() const 
{ return ((_input_control >> Trigger_Shift) & 0xff)-1; }

int PulseConfig::set    () const
{ return ((_input_control >> Set_Shift) & 0xff)-1; }

int PulseConfig::clear  () const
{ return ((_input_control >> Clear_Shift) & 0xff)-1; }


bool PulseConfig::polarity          () const
{ return _output_control & (1<<Polarity_Shift); }

bool PulseConfig::map_set_enable  () const
{ return _output_control & (1<<Map_Set_Ena_Shift); }

bool PulseConfig::map_reset_enable  () const
{ return _output_control & (1<<Map_Reset_Ena_Shift); }

bool PulseConfig::map_trigger_enable() const
{ return _output_control & (1<<Map_Trigger_Ena_Shift); }


unsigned PulseConfig::prescale() const { return _prescale; }

unsigned PulseConfig::delay   () const { return _delay; }

unsigned PulseConfig::width   () const { return _width; }


