#ifndef AmiQt_PeakFitPost_hh
#define AmiQt_PeakFitPost_hh

#include "ami/data/PeakFitPlot.hh"
#include "ami/data/ConfigureRequest.hh"

namespace Ami {
  class Cds;
  class DescEntry;
  class PeakFit;
  namespace Qt {
    class AxisInfo;
    class ChannelDefinition;
    class PeakFitPost {
    public:
      PeakFitPost(unsigned          channel,
		  Ami::PeakFitPlot* desc);
      PeakFitPost(const char*&   p);
      ~PeakFitPost();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels,
		     const AxisInfo&, ConfigureRequest::Source);
    private:
      unsigned   _channel;
      unsigned   _output_signature;
      Ami::PeakFitPlot* _input;
    };
  };
};

#endif
		 
