/*
 * FrameV1.cc
 *
 *  Created on: Nov 6, 2009
 *      Author: jackp
 */

#include "pdsdata/pnCCD/FrameV1.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/ConfigV2.hh"

using namespace Pds;
using namespace PNCCD;

uint32_t FrameV1::specialWord() const { return _specialWord; }

uint32_t FrameV1::frameNumber() const { return _frameNumber; }

uint32_t FrameV1::timeStampHi() const { return _timeStampHi; }

uint32_t FrameV1::timeStampLo() const { return _timeStampLo; }

const uint16_t* FrameV1::data() const {return (const uint16_t*)(this+1);}

unsigned FrameV1::sizeofData(const ConfigV1& cfg) const {
  return (cfg.payloadSizePerLink()-sizeof(*this))/sizeof(uint16_t);
}

const FrameV1* FrameV1::next(const ConfigV1& cfg) const {
  return (const FrameV1*)(((char*)this)+cfg.payloadSizePerLink());
}

unsigned FrameV1::sizeofData(const ConfigV2& cfg) const {
  return (cfg.payloadSizePerLink()-sizeof(*this))/sizeof(uint16_t);
}

const FrameV1* FrameV1::next(const ConfigV2& cfg) const {
  return (const FrameV1*)(((char*)this)+cfg.payloadSizePerLink());
}
