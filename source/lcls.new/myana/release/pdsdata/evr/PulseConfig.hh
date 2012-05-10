#ifndef Evr_PulseConfig_hh
#define Evr_PulseConfig_hh

#include <stdint.h>

namespace Pds {
  namespace EvrData {

    class PulseConfig {
    public:
      PulseConfig ();
      PulseConfig (unsigned pulse,                 // Pulse ID
       int      trigger,               // Pulse input control
       int      set,
       int      clear,
       bool     polarity,              // Pulse output control
       bool     map_set_enable,
       bool     map_reset_enable,
       bool     map_trigger_enable,
       unsigned prescale,              // Pulse parameters
       unsigned delay,
       unsigned width);
    public:
      //  internal pulse generation channel
      unsigned pulse  () const;

      //  id of generated pulse for each mode (edge/level)
      int trigger() const;
      int set    () const;
      int clear  () const;

      //  positive(negative) level
      bool polarity          () const;
      //  enable pulse generation masks
      bool map_set_enable    () const;
      bool map_reset_enable  () const;
      bool map_trigger_enable() const;

      //  pulse event prescale
      unsigned prescale() const;
      //  delay in 119MHz clks
      unsigned delay   () const;
      //  width in 119MHz clks
      unsigned width   () const;
    private:
      uint32_t _pulse;
      uint32_t _input_control;
      uint32_t _output_control;
      uint32_t _prescale;
      uint32_t _delay;
      uint32_t _width;
    };
  };
};

#endif
