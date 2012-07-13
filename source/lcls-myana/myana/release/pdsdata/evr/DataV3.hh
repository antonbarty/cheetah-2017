//
//  Class for configuration of the Event Receiver
//
#ifndef Evr_DataV3_hh
#define Evr_DataV3_hh

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

#pragma pack(4)

namespace Pds
{
namespace EvrData
{

class DataV3
{
  /*
   * Data layout:
   *
   * ---------------
   * Data members in this class
   * --------------- 
   * FIFO Events  (array of class FIFOEvent)
   */  
public:
  enum { Version = 3 };
  
  class FIFOEvent;  
  
  /* public functions*/  
  DataV3(uint32_t u32NumFifoEvents, const FIFOEvent* lFifoEvent);
  DataV3(const DataV3& dataCopy);
      
  uint32_t          numFifoEvents()                     const;
  const FIFOEvent&  fifoEvent(unsigned int iEventIndex) const;

  unsigned int      size()                              const;
    
protected:  
  /*
   * Data members are put in protected section (instead of private section), because 
   * this class will be augmented with utility functions for pds programs.
   */  
  uint32_t _u32NumFifoEvents;    
};

/*
 * Copied from /reg/g/pcds/package/external/evgr_V00-00-02/evgr/evr/evr.hh
 */
class DataV3::FIFOEvent 
{
public:
  uint32_t TimestampHigh;
  uint32_t TimestampLow;
  uint32_t EventCode;
};

} // namespace EvrData
} // namespace Pds

#pragma pack()

#endif
