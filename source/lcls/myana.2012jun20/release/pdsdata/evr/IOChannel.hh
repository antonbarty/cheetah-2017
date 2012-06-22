#ifndef Evr_IOChannel_hh
#define Evr_IOChannel_hh

#include "pdsdata/xtc/DetInfo.hh"

#pragma pack(4)

namespace Pds {
  namespace EvrData {
    class IOChannel {
      enum { MaxInfos=8 };
    public:
      enum { NameLength=12 };
      IOChannel();
      IOChannel(const char*, const Pds::DetInfo*, unsigned);
      IOChannel(const IOChannel&);
    public:
      const char*         name  ()         const;
      unsigned            ninfo ()         const;
      const Pds::DetInfo& info  (unsigned) const;

      unsigned            size  ()         const;
    private:
      char         _name[NameLength];
      uint32_t     _ninfo;
      Pds::DetInfo _info[MaxInfos];
    };
  };
};

#pragma pack()

#endif
