//
//  Class for Process Variable Monitoring
//
#ifndef PdsData_PVMonitor_hh
#define PdsData_PVMonitor_hh

#include <stdint.h>

namespace Pds {

#pragma pack(4)

  namespace ControlData {

    class PVMonitor {
    public:
      enum { NameSize=32 };
    public:
      PVMonitor();
      PVMonitor(const char* pvname, double loValue, double hiValue);
      PVMonitor(const char* pvname, unsigned index, double loValue, double hiValue);
      PVMonitor(const PVMonitor&);
      ~PVMonitor();
    public:
      bool operator<(const PVMonitor&) const;
    public:
      const char* name            () const;
      bool        array           () const;
      unsigned    index           () const;
      double      loValue         () const;
      double      hiValue         () const;
    private:
      char     _name[NameSize];
      uint32_t _index;
      double   _loValue;
      double   _hiValue;
    };

  };

#pragma pack()
};

#endif
