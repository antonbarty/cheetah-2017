#ifndef AmiQt_WaveformClient_hh
#define AmiQt_WaveformClient_hh

#include "ami/qt/Client.hh"

namespace Ami {
  namespace Qt {
    class CursorsX;
    class EdgeFinder;
    class CurveFit;

    class WaveformClient : public Client {
    public:
      WaveformClient(QWidget*,const Pds::DetInfo&, unsigned);
      ~WaveformClient();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      void save_plots(const QString&) const;
    private:
      void _configure(char*& p, 
		      unsigned input, 
		      unsigned& output,
		      ChannelDefinition* ch[], 
		      int* signatures, 
		      unsigned nchannels);
      void _setup_payload(Cds&);
      void _update();
      void _prototype(const DescEntry&);

    private:
      EdgeFinder*        _edges;
      CursorsX*          _cursors;
      CurveFit*          _fits;
      bool               _initialized;
    };
  };
};

#endif
