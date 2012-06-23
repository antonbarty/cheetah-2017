#ifndef Evr_EventCodeV5_hh
#define Evr_EventCodeV5_hh

#include <stdint.h>

#pragma pack(4)

namespace Pds
{
namespace EvrData
{

class EventCodeV5
{
public:
  enum { DescSize = 16 };

  EventCodeV5() {}                         // For array initialization

  EventCodeV5(uint16_t    u16Code,         // Command
	      const char* desc);

  EventCodeV5(uint16_t    u16Code,         // Readout code
	      const char* desc,
	      uint32_t    u32MaskTrigger,
	      uint32_t    u32MaskSet    , 
	      uint32_t    u32MaskClear);
    
  EventCodeV5(uint16_t    u16Code,         // External control
	      const char* desc,
	      bool        bLatch,
	      uint32_t    u32ReportDelay = 0,
	      uint32_t    u32ReportWidth = 1);

  EventCodeV5(uint16_t    u16Code,         // Generic
	      const char* desc,
	      bool        bReadout,
	      bool        bCommand,    
	      bool        bLatch,
	      uint32_t    u32ReportDelay = 0,
	      uint32_t    u32ReportWidth = 1,
	      uint32_t    u32MaskTrigger = 0,
	      uint32_t    u32MaskSet     = 0, 
	      uint32_t    u32MaskClear   = 0);
    
  EventCodeV5(const EventCodeV5&);

  uint16_t    code      () const { return _u16Code; }
  const char* desc      () const { return _desc; }

  bool      isReadout   () const { return ( _u16MaskEventAttr & (1<<EventAttrBitReadout) )     != 0; }
  bool      isCommand   () const { return ( _u16MaskEventAttr & (1<<EventAttrBitCommand) )     != 0; }
  bool      isLatch     () const { return ( _u16MaskEventAttr & (1<<EventAttrBitLatch) )       != 0; }

  uint32_t  reportDelay () const { return _u32ReportDelay; }
  uint32_t  reportWidth () const { return _u32ReportWidth; }
  uint32_t  releaseCode () const { return _u32ReportWidth; }

  uint32_t  maskTrigger () const { return _u32MaskTrigger; }
  uint32_t  maskSet     () const { return _u32MaskSet; }
  uint32_t  maskClear   () const { return _u32MaskClear; }

private:
  enum EventAttrBitEnum { EventAttrBitReadout = 0, EventAttrBitCommand = 1, EventAttrBitLatch = 2 };
    
  uint16_t _u16Code;
  uint16_t _u16MaskEventAttr;
  uint32_t _u32ReportDelay;
  uint32_t _u32ReportWidth;
  uint32_t _u32MaskTrigger;
  uint32_t _u32MaskSet;
  uint32_t _u32MaskClear;
  char     _desc[DescSize];
};

} // namespace EvrData
} // namespace Pds

#pragma pack()

#endif
