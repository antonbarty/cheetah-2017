#ifndef AmiQt_CursorPost_hh
#define AmiQt_CursorPost_hh

#include "ami/data/ConfigureRequest.hh"

namespace Ami {
  class Cds;
  class DescEntry;
  class BinMath;
  namespace Qt {
    class AxisInfo;
    class ChannelDefinition;
    class CursorDefinition;
    class CursorPost {
    public:
      CursorPost(unsigned       channel,
		 BinMath*       desc);
      CursorPost(const char*&   p);
      ~CursorPost();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels,
		     const AxisInfo&, ConfigureRequest::Source);
    private:
      unsigned _channel;
      unsigned _output_signature;
      BinMath* _input;
    };
  };
};

#endif
		 
