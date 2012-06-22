//
//  class for configuring online frame feature extraction process
//
#ifndef Camera_FrameFexConfigV1_hh
#define Camera_FrameFexConfigV1_hh

#include "pdsdata/camera/FrameCoord.hh"

namespace Pds {
  namespace Camera {

    class FrameFexConfigV1 {
    public:
      enum { Version=1 };
      enum Forwarding { NoFrame, FullFrame, RegionOfInterest };
      enum Processing { NoProcessing, GssFullFrame, GssRegionOfInterest, GssThreshold };

      FrameFexConfigV1();
      FrameFexConfigV1( Forwarding        forwarding,
			unsigned          fwd_prescale,
			Processing        processing,
			const FrameCoord& roiBegin,
			const FrameCoord& roiEnd,
			unsigned          threshold,
			unsigned          masked_pixels,
			const FrameCoord* masked_coords );

      FrameFexConfigV1(const FrameFexConfigV1&);

      //  forwarding policy for frame data
      Forwarding        forwarding() const;

      //  prescale of events with forwarded frames
      unsigned          forward_prescale() const;

      //  algorithm to apply to frames to produce processed output
      Processing        processing() const;

      //  coordinate of start of rectangular region of interest (inclusive)
      const FrameCoord& roiBegin  () const;

      //  coordinate of finish of rectangular region of interest (exclusive)
      const FrameCoord& roiEnd    () const;

      //  pixel data threshold value to apply in processing 
      unsigned          threshold () const;

      //  count of masked pixels to exclude from processing
      unsigned          number_of_masked_pixels () const;

      //  location of masked pixel coordinates 
      //  appended to the end of this structure
      const FrameCoord& masked_pixel_coordinates() const;

      //  size of this structure
      //  (including appended masked pixel coordinates)
      unsigned size() const;

    private:
      uint32_t   _forwarding;       // frame forwarding policy
      uint32_t   _forward_prescale; // event prescale for forwarding
      uint32_t   _processing;       // processing algorithm 
      FrameCoord _roiBegin;         // starting coord of ROI (inclusive)
      FrameCoord _roiEnd;           // finishing coord of ROI (exclusive)
      uint32_t   _threshold;        // threshold value for processing pixel data
      uint32_t   _masked_pixel_count; // count of pixels masked from processing
    };

  };
};
#endif
