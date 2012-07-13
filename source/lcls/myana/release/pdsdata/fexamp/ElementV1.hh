//
//  Class for rectangular frame data
//
#ifndef Pds_FEXAMP_ElementV1_hh
#define Pds_FEXAMP_ElementV1_hh

#include "pdsdata/fexamp/ElementHeader.hh"
#include "pdsdata/fexamp/ConfigV1.hh"

#include <stdint.h>

namespace Pds {

  namespace Fexamp {

    class ConfigV1;
    typedef Pds::Fexamp::ConfigV1  FexampConfigType;

    class FexampWord {
      public:
        FexampWord() {};
        ~FexampWord() {};
        uint32_t pumpbits() { return (_value >> 14) & 0xf; }
        uint32_t adcbits()  { return (_value & 0x3fff); }
      private:
        uint32_t _value;
    };

    class ElementV1 : public ElementHeader {
      public:
        enum {Version=1};
        enum dimensions {Uint32sPerSingleSampleArray=1024, Uint32sInLastWords=2};

        ElementV1() {};
        ~ElementV1() {};

        FexampWord getRawWord(unsigned index) {
          FexampWord* dat = (FexampWord*) (this+1);
          return dat[index];
        }

        uint32_t penultimateWord(FexampConfigType* c) {
          unsigned size = ElementV1::Uint32sPerSingleSampleArray * ((c->get(FexampConfigType::PerMclkCount)+1)>>4);
          uint32_t* u = (uint32_t*) (this+1);
          return u[size];
        }

        uint32_t ultimateWord(FexampConfigType* c) {
          unsigned size = ElementV1::Uint32sPerSingleSampleArray * ((c->get(FexampConfigType::PerMclkCount)+1)>>4);
          uint32_t* u = (uint32_t*) (this+1);
          return u[size+1];
        }

        unsigned numberOfFexampWords(FexampConfigType* c) {
          return (ElementV1::Uint32sPerSingleSampleArray * ((c->get(FexampConfigType::PerMclkCount)+1)>>4));
        }

        uint32_t penultimateWord(FexampConfigType& c) {
          unsigned size = ElementV1::Uint32sPerSingleSampleArray * ((c.get(FexampConfigType::PerMclkCount)+1)>>4);
          uint32_t* u = (uint32_t*) (this+1);
          return u[size];
        }

        uint32_t ultimateWord(FexampConfigType& c) {
          unsigned size = ElementV1::Uint32sPerSingleSampleArray * ((c.get(FexampConfigType::PerMclkCount)+1)>>4);
          uint32_t* u = (uint32_t*) (this+1);
          return u[size+1];
        }

        unsigned numberOfFexampWords(FexampConfigType& c) {
          return (ElementV1::Uint32sPerSingleSampleArray * ((c.get(FexampConfigType::PerMclkCount)+1)>>4));
        }
    };
  };
};

#endif
