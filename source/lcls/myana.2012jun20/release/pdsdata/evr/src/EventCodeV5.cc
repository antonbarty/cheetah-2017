#include "pdsdata/evr/EventCodeV5.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

EventCodeV5::EventCodeV5(uint16_t    u16Code,
			 const char* desc) :
  _u16Code          (u16Code),
  _u16MaskEventAttr ((1 << EventAttrBitCommand)),
  _u32ReportDelay   (0),
  _u32ReportWidth   (1),
  _u32MaskTrigger   (0),
  _u32MaskSet       (0),
  _u32MaskClear     (0)
{
  strncpy(_desc, desc, DescSize-1);
  _desc[DescSize-1]=0;
}

EventCodeV5::EventCodeV5(uint16_t    u16Code,
			 const char* desc,
			 uint32_t    u32MaskTrigger,
			 uint32_t    u32MaskSet, 
			 uint32_t    u32MaskClear ) :
  _u16Code          (u16Code),
  _u16MaskEventAttr ((1 << EventAttrBitReadout)),
  _u32ReportDelay   (0),
  _u32ReportWidth   (1),
  _u32MaskTrigger   (u32MaskTrigger),
  _u32MaskSet       (u32MaskSet),
  _u32MaskClear     (u32MaskClear)
{
  strncpy(_desc, desc, DescSize-1);
  _desc[DescSize-1]=0;
}

EventCodeV5::EventCodeV5(uint16_t    u16Code,
			 const char* desc,
			 bool        bLatch,
			 uint32_t    u32ReportDelay,
			 uint32_t    u32ReportWidth ) :
  _u16Code          (u16Code),
  _u16MaskEventAttr ((bLatch?       (1 << EventAttrBitLatch)      :0)),
  _u32ReportDelay   (u32ReportDelay),
  _u32ReportWidth   (u32ReportWidth),
  _u32MaskTrigger   (0),
  _u32MaskSet       (0),
  _u32MaskClear     (0)
{
  strncpy(_desc, desc, DescSize-1);
  _desc[DescSize-1]=0;
}

EventCodeV5::EventCodeV5(uint16_t    u16Code,
			 const char* desc,
			 bool        bReadout,
			 bool        bCommand,
			 bool        bLatch,
			 uint32_t    u32ReportDelay,
			 uint32_t    u32ReportWidth,  
			 uint32_t    u32MaskTrigger,
			 uint32_t    u32MaskSet, 
			 uint32_t    u32MaskClear ) :
  _u16Code          (u16Code),
  _u16MaskEventAttr ((bReadout?    (1 << EventAttrBitReadout)    :0) | 
                     (bCommand?    (1 << EventAttrBitCommand)    :0) |
                     (bLatch?      (1 << EventAttrBitLatch)      :0)),
  _u32ReportDelay   (u32ReportDelay),
  _u32ReportWidth   (u32ReportWidth),
  _u32MaskTrigger   (u32MaskTrigger),
  _u32MaskSet       (u32MaskSet),
  _u32MaskClear     (u32MaskClear)
{
  strncpy(_desc, desc, DescSize-1);
  _desc[DescSize-1]=0;
}

EventCodeV5::EventCodeV5(const EventCodeV5& c) :
  _u16Code          (c._u16Code),
  _u16MaskEventAttr (c._u16MaskEventAttr),
  _u32ReportDelay   (c._u32ReportDelay),
  _u32ReportWidth   (c._u32ReportWidth),
  _u32MaskTrigger   (c._u32MaskTrigger),
  _u32MaskSet       (c._u32MaskSet),
  _u32MaskClear     (c._u32MaskClear)
{
  strncpy(_desc, c._desc, DescSize-1);
  _desc[DescSize-1]=0;
}
