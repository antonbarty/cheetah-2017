//
//  Class for Process Variable Control
//
#ifndef PdsData_PVControl_hh
#define PdsData_PVControl_hh

#include <stdint.h>

namespace Pds {

#pragma pack(4)

  namespace ControlData {

    class PVControl {
    public:
      enum { NameSize=32 };
    public:
      PVControl();
      PVControl(const char* pvname, double setValue);
      PVControl(const char* pvname, unsigned index, double setValue);
      PVControl(const PVControl&);
      ~PVControl();
    public:
      bool operator<(const PVControl&) const;
    public:
      const char* name () const;
      bool        array() const;
      unsigned    index() const;
      double      value() const;
    private:
      char     _name[NameSize];
      uint32_t _index;
      double   _value;
    };

  };

#pragma pack()
};

#endif
