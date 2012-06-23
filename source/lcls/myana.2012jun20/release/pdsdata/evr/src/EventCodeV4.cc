#include "pdsdata/evr/EventCodeV4.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

EventCodeV4::EventCodeV4(
  uint16_t u16Code,
  bool     bReadout,
  bool     bTerminator,
  uint32_t u32ReportDelay,
  uint32_t u32ReportWidth,  
  uint32_t u32MaskTrigger,
  uint32_t u32MaskSet, 
  uint32_t u32MaskClear
  ) :
  _u16Code          (u16Code),
  _u16MaskEventAttr ((bReadout?    (1 << EventAttrBitReadout)    :0) | 
                     (bTerminator? (1 << EventAttrBitTerminator) :0)),
  _u32ReportDelay   (u32ReportDelay),
  _u32ReportWidth   (u32ReportWidth),
  _u32MaskTrigger   (u32MaskTrigger),
  _u32MaskSet       (u32MaskSet),
  _u32MaskClear     (u32MaskClear)
{
}
