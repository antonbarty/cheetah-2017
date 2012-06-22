#ifndef AmiQt_CurveFitPost_hh
#define AmiQt_CurveFitPost_hh

#include "ami/data/CurveFit.hh"
#include "ami/data/ConfigureRequest.hh"

namespace Ami {
  class Cds;
  class DescEntry;
  class CurveFit;
  namespace Qt {
    class AxisInfo;
    class ChannelDefinition;
    class CurveFitPost {
    public:
      CurveFitPost(unsigned          channel,
		   Ami::CurveFit*    input);
      CurveFitPost(const char*&   p);
      ~CurveFitPost();
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
      Ami::CurveFit* _input;
    };
  };
};

#endif
		 
