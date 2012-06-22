#ifndef ACQIRISDATADESC_HH
#define ACQIRISDATADESC_HH

#include <stdint.h>
#include "pdsdata/xtc/TypeId.hh"
#include "ConfigV1.hh"

// the layout of acqiris data in memory is as follows:
// channel 1:
//  - data descriptor
//  - segment descriptor(s)
//  - waveform data
// and subsequent channels are identical to the above.
// notes:
// 1) the location in memory of the waveform data as well
// as the next channel depends on the number of segments, as well
// as the number of samples per segment.  so some of the methods
// to compute addresses require the horizontal configuration 
// parameters as input.
// 2) the memory is laid out in a "fixed" way depending on the
// expected number of segments/samples.  if the acqiris returns
// something different than expected, that will show up in the
// data/segment descriptor information, but will NOT change the
// memory map of the data (which is determined from the "expected" values)
// 3) two acqiris structs have been copied to break the dependency on
// their software (see comments in code for details).  The segment descriptor
// copy is tricky, since it's structure depends on the read-mode, and
// I'm not going to take the time now to figure out how to support
// read-modes that we don't currently use.
// 4) the padding required by the DMA engine may have to change if the
// readmode changes.  This is the variable "extra" in AcqDataDesc.
// - cpo, Mar 4, 2008

#include <stdint.h>

namespace Pds {

  namespace Acqiris {

    class TimestampV1 {
    public:
      double   pos  () const;
      uint64_t value() const;
      uint64_t operator-(const TimestampV1& ts) const;
    private:
      // this is a copy of AqSegmentDescriptor in AcqirisDataTypes.h
      // to break dependencies with acqiris software.  Note that if
      // the readmode changes, then this structure should be changed.
      double   _horPos;
      uint32_t _timeStampLo;
      uint32_t _timeStampHi;
    };

    class DataDescV1 {
    public:
      uint32_t nbrSamplesInSeg() const;
      uint32_t nbrSegments() const;
      TimestampV1& timestamp(uint32_t seg);
      int16_t* waveform(const HorizV1& hconfig);
      uint32_t indexFirstPoint();
      DataDescV1* nextChannel(const HorizV1& hconfig);

      static uint32_t timestampSize(const HorizV1& hconfig);
      static uint32_t waveformSize(const HorizV1& hconfig);
      static uint32_t totalSize(const HorizV1& hconfig);
      static Pds::TypeId typeId();
      enum {NumberOfBits=10};
      enum {BitShift=6};
    private:
      // extra space required by Acqiris DMA engine.  may need
      // to change if readmode changes.
      static const uint32_t _extra=32*sizeof(short);
      TimestampV1* _timestamp();

      // this is a copy of AqDataDescriptor in AcqirisDataTypes.h
      // to break dependencies with acqiris software.
      uint32_t _returnedSamplesPerSeg;
      uint32_t _indexFirstPoint; // indexFirstPoint is the first valid point. 
      //!< Note: Not valid for 'readMode = ReadModeSeqRawW'.
      double   _sampTime;
      double   _vGain;
      double   _vOffset;
      uint32_t _returnedSegments;//!< When reading multiple segments in one waveform
      uint32_t _nbrAvgWforms;        
      uint32_t _actualTriggersInAcqLo;
      uint32_t _actualTriggersInAcqHi;
      uint32_t _actualDataSize;
      uint32_t _reserved2;    
      double   _reserved3;
    };
  }
}

#endif
