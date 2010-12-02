#include <stdio.h>
#include <memory.h>

#include "pdsdata/evr/DataV3.hh"

using namespace Pds;
using namespace EvrData;

DataV3::DataV3(uint32_t u32NumFifoEvents, const FIFOEvent* lFifoEvent) : _u32NumFifoEvents(u32NumFifoEvents)
{
  if ( u32NumFifoEvents > 0 )
  {
    char *next = (char*) (this + 1);
    memcpy(next, lFifoEvent, _u32NumFifoEvents * sizeof(FIFOEvent));  
  }
}

DataV3::DataV3(const DataV3& dataCopy)
{
  _u32NumFifoEvents = dataCopy.numFifoEvents();
  
  const char *src = (char*) (&dataCopy + 1);
  char *      dst = (char*) (this + 1);
  memcpy(dst, src, _u32NumFifoEvents * sizeof(FIFOEvent));  
}

uint32_t DataV3::numFifoEvents() const
{
  return _u32NumFifoEvents; 
}

const DataV3::FIFOEvent& DataV3::fifoEvent(unsigned int uEventIndex)  const
{
  const FIFOEvent *lFifoEvent = reinterpret_cast < const FIFOEvent * >(this + 1);
  return lFifoEvent[uEventIndex];
}

unsigned int DataV3::size() const
{
  return ( sizeof(*this) + _u32NumFifoEvents * sizeof(FIFOEvent) );
}
