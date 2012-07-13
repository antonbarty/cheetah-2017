#ifndef ACQIRISCONFIGV1_HH
#define ACQIRISCONFIGV1_HH

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

namespace Pds {

  namespace Acqiris {
#pragma pack(4)
    class VertV1 {
    public:
      VertV1();
      VertV1(double fullScale, double offset,
             uint32_t coupling, uint32_t bandwidth);
      double   fullScale() const;
      double   offset()    const;
      uint32_t coupling()  const;
      uint32_t bandwidth() const;
      double   slope()     const;
      void     dump()      const;
      enum Coupling  {GND,DC,AC,DC50ohm,AC50ohm};
      enum Bandwidth {None,MHz25,MHz700,MHz200,MHz20,MHz35};
    private:
      double   _fullScale;
      double   _offset;
      uint32_t _coupling;
      uint32_t _bandwidth;
    };

    class HorizV1 {
    public:
      HorizV1();
      HorizV1(double   sampInterval,
              double   delayTime,
              uint32_t nbrSamples,
              uint32_t nbrSegments);
      double   sampInterval()     const;
      double   delayTime()        const;
      uint32_t nbrSamples()       const;
      uint32_t nbrSegments()      const;
      void     dump()             const;
    private:
      double   _sampInterval;
      double   _delayTime;
      uint32_t _nbrSamples;
      uint32_t _nbrSegments;
    };

    class TrigV1 {
    public:
      TrigV1();
      TrigV1(uint32_t coupling,
             uint32_t input,
             uint32_t slope,
             double   level);
      uint32_t coupling()     const;
      uint32_t input()        const;
      uint32_t slope()        const;
      double   level()        const;
      void     dump()         const;
      enum Source   {Internal=1,External=-1};
      enum Coupling {DC=0,AC=1,HFreject=2,DC50ohm=3,AC50ohm=4};
      enum Slope    {Positive,Negative,OutOfWindow,IntoWindow,HFDivide,SpikeStretcher};
    private:
      uint32_t _coupling;
      uint32_t _input;
      uint32_t _slope;
      double   _level;
    };

    class ConfigV1 {
    public:
      enum {Version=1};
      enum {MaxChan=20};
      ConfigV1 ();
      ~ConfigV1 ();
      ConfigV1 (uint32_t nbrConvertersPerChannel,
                uint32_t channelMask,
                uint32_t nbrBanks,
                const TrigV1& trig,
                const HorizV1& horiz,
                const VertV1* chanConfig);
      uint32_t nbrConvertersPerChannel() const;
      uint32_t channelMask()      const;
      uint32_t nbrChannels()      const;
      uint32_t nbrBanks()         const;
      HorizV1& horiz();
      TrigV1& trig();
      VertV1& vert(uint32_t channel);
      const HorizV1& horiz() const;
      const TrigV1& trig() const;
      const VertV1& vert(uint32_t channel) const;
      static Pds::TypeId typeId();

      void dump() const;
    private:
      uint32_t _nbrConvertersPerChannel;
      uint32_t _channelMask;
      uint32_t _nbrBanks;
      TrigV1   _trig;
      HorizV1  _horiz;
      VertV1   _vert[MaxChan];
    };
#pragma pack()
  }
}

#endif
