/*************************************************************************

 As specified in SIS Documentation HLL Framegenerator secion 5.1.1 x/y dimension
 
*************************************************************************/


#ifndef FORMAT_H
#define FORMAT_H

namespace Pds {
  /* structure type for frame header, contains static information for each frame */
  typedef	struct
  {
      uint32_t    specialWord;
      uint32_t    frameNumber;
      uint32_t    TimeStampHi;   // make this a unit64_t when we know the endianess.
      uint32_t    TimeStampLo;
  } PnccdFrameHeaderType;	        // size is 16 bytes */
}
#endif // FORMAT_H

