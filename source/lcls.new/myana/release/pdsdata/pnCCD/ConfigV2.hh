#ifndef PNCCD_ConfigV2_hh
#define PNCCD_ConfigV2_hh

#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

namespace Pds {

  namespace PNCCD {
    using std::string;

    class ConfigV2 {
    public:
      enum { Version=2 };
      ConfigV2();
      ConfigV2(string);
      uint32_t numLinks()             const {return _numLinks;}
      uint32_t payloadSizePerLink()   const {return _payloadSizePerLink;} // in bytes
      uint32_t numChannels()          const {return _numChannels;}
      uint32_t numRows()              const {return _numRows;}
      uint32_t numSubmoduleChannels() const {return _numSubmoduleChannels;}
      uint32_t numSubmoduleRows()     const {return _numSubmoduleRows;}
      uint32_t numSubmodules()        const {return _numSubmodules;}
      uint32_t camexMagic()           const {return _camexMagic;}
      const char* info()              const {return _info;}
      const char* timingFName()       const {return _timingFName;}
      unsigned size()                 const {return sizeof(*this);}
    private:
      uint32_t _numLinks;
      uint32_t _payloadSizePerLink;
      uint32_t _numChannels;
      uint32_t _numRows;
      uint32_t _numSubmoduleChannels;
      uint32_t _numSubmoduleRows;
      uint32_t _numSubmodules;
      uint32_t _camexMagic;
      char _info[256];
      char _timingFName[256];
    };

  };
};

#endif
