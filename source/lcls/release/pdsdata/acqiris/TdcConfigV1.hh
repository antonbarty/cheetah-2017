#ifndef ACQIRISTDCCONFIGV1_HH
#define ACQIRISTDCCONFIGV1_HH

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

namespace Pds {

  namespace Acqiris {
#pragma pack(4)
    class TdcChannel { 
    public:
      enum Channel  {Veto=-2, Common=-1, 
		     Input1=1, Input2=2, Input3=3, 
		     Input4=4, Input5=5, Input6=6};
      enum Mode     {Active=0,Inactive=1};
      enum Slope    {Positive,Negative};
    public:
      TdcChannel();
      TdcChannel(Channel  chan,
		 Mode     mode,
		 Slope    slope,
		 double   level);
    public:
      Channel  channel()      const;
      Mode     mode   ()      const;
      Slope    slope  ()      const;
      double   level  ()      const;
    private:
      uint32_t _channel;
      uint32_t _mode;
      double   _level;
    };

    class TdcAuxIO {
    public:
      enum Channel     {IOAux1=1, IOAux2=2};
      enum Mode        {BankSwitch=1, Marker=2,
			OutputLo=32 , OutputHi=33};
      enum Termination {ZHigh=0, Z50=1};
    public:
      TdcAuxIO();
      TdcAuxIO(Channel     channel,
	       Mode        mode,
	       Termination term);
    public:
      Channel     channel() const;
      Mode        mode   () const;
      Termination term   () const;
    private:
      uint32_t _channel;
      uint32_t _signal;
      uint32_t _qualifier;
    };

    class TdcVetoIO {
    public:
      enum Channel     {ChVeto=13};
      enum Mode        {Veto=1, SwitchVeto=2,
			InvertedVeto=3, InvertedSwitchVeto=4};
      enum Termination {ZHigh=0, Z50=1};
    public:
      TdcVetoIO();
      TdcVetoIO(Mode        mode,
		Termination term);
    public:
      Channel     channel() const;
      Mode        mode   () const;
      Termination term   () const;
    private:
      uint32_t _channel;
      uint32_t _signal;
      uint32_t _qualifier;
    };

    class TdcConfigV1 {
    public:
      enum {Version=1};
      enum {NChannels=8};
      enum {NAuxIO=2};
    public:
      TdcConfigV1 ();
      ~TdcConfigV1 ();
      TdcConfigV1 (const TdcChannel channels[],
		   const TdcAuxIO   auxio[],
		   const TdcVetoIO& veto);
    public:
      const TdcChannel& channel(unsigned) const;
      const TdcAuxIO&   auxio  (unsigned) const;
      const TdcVetoIO&  veto   ()         const;
    private:
      TdcChannel _channel [NChannels];
      TdcAuxIO   _auxIO   [NAuxIO];
      TdcVetoIO  _veto;
    };
#pragma pack()
  }
}

#endif
