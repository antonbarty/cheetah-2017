#ifndef Pds_DESCENTRY_HH
#define Pds_DESCENTRY_HH

#include "ami/data/Desc.hh"

#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {

  class DescEntry : public Desc {
  public:
    enum Type {Scalar, TH1F, TH2F, Prof, Image, Waveform, Scan, Ref, Cache};
    Type type() const;

    const Pds::DetInfo& info()    const;
    unsigned            channel() const;

    const char* xtitle() const;
    const char* ytitle() const;
    const char* zunits() const;
    unsigned short size() const;

    bool isnormalized() const;
    bool aggregate   () const;
    bool isweighted_type() const;
    bool hasPedCalib () const;
    bool hasGainCalib() const;
    bool hasRmsCalib () const;
    unsigned options() const { return _options>>User; }


    void normalize(bool);
    void aggregate(bool);
    void pedcalib (bool);
    void gaincalib(bool);
    void rmscalib (bool);
    void options  (unsigned);

    void xwarnings(float warn, float err);
    bool xhaswarnings() const;
    float xwarn() const;
    float xerr() const;

    void ywarnings(float warn, float err);
    bool yhaswarnings() const;
    float ywarn() const;
    float yerr() const;

  protected:
    DescEntry(const char* name, const char* xtitle, const char* ytitle, 
	      Type type, unsigned short size, bool isnormalized=true, bool aggregate=true,
              unsigned options=0);

    DescEntry(const Pds::DetInfo& info, unsigned channel,
	      const char* name, const char* xtitle, const char* ytitle, 
	      Type type, unsigned short size, bool isnormalized=true, bool aggregate=true,
              unsigned options=0);

    DescEntry(const Pds::DetInfo& info, unsigned channel,
	      const char* name, const char* xtitle, const char* ytitle, const char* zunits,
	      Type type, unsigned short size, bool isnormalized=true, bool aggregate=true,
              bool hasPedCalib=false, bool hasGainCalib=false, bool hasRmsCalib=false,
              unsigned options=0);

  private:
    Pds::DetInfo _info;
    uint32_t     _channel;
    enum {TitleSize=64};
    char _xtitle[TitleSize];
    char _ytitle[TitleSize];
    char _zunits[TitleSize];
    int16_t   _group;
    int16_t   _options;
    uint16_t  _type;
    uint16_t  _size;
    float _xwarn;
    float _xerr;
    float _ywarn;
    float _yerr;

  private:
    enum Option {Normalized, Aggregate, XWarnings, YWarnings, 
                 CalibMom0, CalibMom1, CalibMom2,  // has calibrations of these orders { Ped, Gain, Sigma }
                 User};
  };
};

#endif
