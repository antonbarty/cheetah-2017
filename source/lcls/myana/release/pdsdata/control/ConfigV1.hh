//
//  Class for DAQ Control
//
#ifndef Pds_ConfigV1_hh
#define Pds_ConfigV1_hh

#include <list>
#include <stdint.h>

#include "pdsdata/xtc/ClockTime.hh"
using Pds::ClockTime;

namespace Pds {

  namespace ControlData {

    class PVControl;
    class PVMonitor;

    class ConfigV1 {
    public:
      enum { Version=1 };
    public:
      enum Initialize { Default };
      ConfigV1();
      ConfigV1(Initialize);
      ConfigV1(const std::list<PVControl>&, const std::list<PVMonitor>&);
      ConfigV1(const std::list<PVControl>&, const std::list<PVMonitor>&, const ClockTime&);
      ConfigV1(const std::list<PVControl>&, const std::list<PVMonitor>&, unsigned events );
      ConfigV1(const ConfigV1&);
    private:
      ~ConfigV1();  // Should not be built on the stack (placement new only)
    public:
      bool             uses_duration()       const;
      bool             uses_events  ()       const;
      const ClockTime& duration   ()         const;
      unsigned         events     ()         const;
      unsigned         npvControls()         const;
      const PVControl& pvControl  (unsigned) const;
      unsigned         npvMonitors()         const;
      const PVMonitor& pvMonitor  (unsigned) const;
      unsigned         size       ()         const;
    private:
      uint32_t  _control;
      uint32_t  _reserved;
      ClockTime _duration;
      uint32_t  _npvControls;
      uint32_t  _npvMonitors;
    };

  };

};

#endif
