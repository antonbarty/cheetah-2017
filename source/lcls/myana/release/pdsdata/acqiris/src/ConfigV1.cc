
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include <stdio.h>

using namespace Pds;

using namespace Acqiris;

static const unsigned Version=1;

Pds::TypeId ConfigV1::typeId() {
  return Pds::TypeId(Pds::TypeId::Id_AcqConfig,Version);
}

ConfigV1::ConfigV1() {}

ConfigV1::~ConfigV1() {}

void ConfigV1::dump() const {
  printf("------Acqiris Config-------------\n");
  printf("Channel mask 0x%x\n",channelMask());
  printf("Number of converters per channel %d\n",nbrConvertersPerChannel());
  printf("------Trigger--------------------\n");
  trig().dump();
  printf("------Horizontal-----------------\n");
  horiz().dump();
  for (unsigned i=0;i<nbrChannels();i++) {
    printf("----Vertical (Channel %d)--------\n",i);
    vert(i).dump();
  }
}

HorizV1::HorizV1() {}

HorizV1::HorizV1(double   sampInterval,
                 double   delayTime,   
                 uint32_t nbrSamples,  
                 uint32_t nbrSegments) :
  _sampInterval(sampInterval),
  _delayTime   (delayTime),   
  _nbrSamples  (nbrSamples),  
  _nbrSegments (nbrSegments)
{}

double   HorizV1::sampInterval() const {return _sampInterval;}
double   HorizV1::delayTime()    const {return _delayTime;}
uint32_t HorizV1::nbrSamples()   const {return _nbrSamples;}
uint32_t HorizV1::nbrSegments()  const {return _nbrSegments;}

void HorizV1::dump() const {
  printf("Sample time %e seconds\n",_sampInterval);
  printf("Delay %f\n",_delayTime);
}

TrigV1::TrigV1() {}

TrigV1::TrigV1(uint32_t coupling,
               uint32_t input,   
               uint32_t slope,   
               double   level) :
  _coupling(coupling),
  _input   (input),   
  _slope   (slope),   
  _level   (level)
{}

uint32_t TrigV1::coupling() const {return _coupling;}
uint32_t TrigV1::input()    const {return _input;}
uint32_t TrigV1::slope()    const {return _slope;}
double   TrigV1::level()    const {return _level;}

void TrigV1::dump() const {
  printf("Coupling %d\n",_coupling);
  printf("Input %d\n",_input);
  printf("Slope %d\n",_slope);
  printf("Level %e volts\n",_level);
}

VertV1::VertV1() {}
VertV1::VertV1(double   fullScale,
               double   offset,
               uint32_t coupling,    
               uint32_t bandwidth) :
  _fullScale(fullScale),  _offset(offset),
  _coupling(coupling), _bandwidth(bandwidth) {}

ConfigV1::ConfigV1 (uint32_t nbrConvertersPerChannel,
                    uint32_t channelMask,
                    uint32_t nbrBanks,
                    const TrigV1& trig,
                    const HorizV1& horiz,
                    const VertV1* vert) :
  _nbrConvertersPerChannel(nbrConvertersPerChannel),
  _channelMask(channelMask),_nbrBanks(nbrBanks),_trig(trig),
  _horiz(horiz) {
  for (unsigned i=0;i<MaxChan;i++) {
    _vert[i]=vert[i];
  }
}

void VertV1::dump() const {
  printf("Fullscale %f\n",_fullScale);
  printf("Offset %f\n",_offset);
  printf("Coupling %d\n",_coupling);
  printf("Bandwidth %d\n",_bandwidth);
}

// this constants reflects two things:
// 1) the fact that 511 ADC counts = +0.5*fullscale-1bit
//    and -512 ADC counts = -0.5*fullscale
//    since the value read out is a signed 10-bit number,
//    and since the fullscale voltage covers -512 to +511.
// 2) the 10-bit acqiris data is shifted left by 6 bits,
//    presumably to take advantage of improved CPU performance
//    using signed arithmetic to compute voltages.
static const unsigned normalize=
(1<<Acqiris::DataDescV1::NumberOfBits)*(1<<Acqiris::DataDescV1::BitShift);

double   VertV1::slope() const {return _fullScale/(float)(normalize);}
double   VertV1::fullScale() const {return _fullScale;}
double   VertV1::offset()    const {return _offset;}
uint32_t VertV1::coupling()  const {return _coupling;}
uint32_t VertV1::bandwidth() const {return _bandwidth;}

uint32_t ConfigV1::nbrConvertersPerChannel() const {return _nbrConvertersPerChannel;}

uint32_t ConfigV1::channelMask() const {return _channelMask;}

uint32_t ConfigV1::nbrChannels() const {
  uint32_t numchan=0;
  for (unsigned i=0;i<32;i++) if (_channelMask&(1<<i)) numchan++;
  return numchan;
}

uint32_t ConfigV1::nbrBanks() const {return _nbrBanks;}

HorizV1& ConfigV1::horiz() {return _horiz;}

TrigV1& ConfigV1::trig() {return _trig;}

VertV1& ConfigV1::vert(uint32_t channel) {return _vert[channel];}

const HorizV1& ConfigV1::horiz() const {return _horiz;}

const TrigV1& ConfigV1::trig() const {return _trig;}

const VertV1& ConfigV1::vert(uint32_t channel) const {return _vert[channel];}
