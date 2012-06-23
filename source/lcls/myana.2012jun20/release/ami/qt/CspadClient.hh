#ifndef AmiQt_CspadClient_hh
#define AmiQt_CspadClient_hh

#include "ami/qt/ImageClient.hh"

class QCheckBox;

namespace Ami {
  namespace Qt {
    class CspadClient : public ImageClient {
      Q_OBJECT
    public:
      CspadClient(QWidget*,const Pds::DetInfo&, unsigned);
      ~CspadClient();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    protected:
      void _configure(char*& p, 
		      unsigned input, 
		      unsigned& output,
		      ChannelDefinition* ch[], 
		      int* signatures, 
		      unsigned nchannels);
      void _setup_payload(Cds&);
    public slots:
      void write_pedestals();
    private:
      //  Specialization widgets
      QCheckBox* _fnBox;
      QCheckBox* _spBox;
    };
  };
};

#endif
      
