#ifndef Ami_EntryTerm_hh
#define Ami_EntryTerm_hh

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryWaveform.hh"

#include "ami/data/Expression.hh"

namespace Ami {
  class EntryWaveformTerm : public Term {
  public:
    EntryWaveformTerm(const EntryWaveform& e,
		      unsigned& index) : _entry(e), _index(index) {}
    ~EntryWaveformTerm() {}
  public:
    double evaluate() const { 
      double n = _entry.info(EntryWaveform::Normalization);
      return (n > 1) ? _entry.content(_index)/n : _entry.content(_index); }
  private:
    const EntryWaveform& _entry;
    unsigned& _index;
  };

  class EntryTH1FTerm : public Term {
  public:
    EntryTH1FTerm(const EntryTH1F& e,
		  unsigned& index) : _entry(e), _index(index) {}
    ~EntryTH1FTerm() {}
  public:
    double evaluate() const { return _entry.content(_index); }
  private:
    const EntryTH1F& _entry;
    unsigned& _index;
  };

  class EntryProfTerm : public Term {
  public:
    EntryProfTerm(const EntryProf& e,
		  unsigned& index) : _entry(e), _index(index) {}
    ~EntryProfTerm() {}
  public:
    double evaluate() const { return _entry.ymean(_index); }
  private:
    const EntryProf& _entry;
    unsigned& _index;
  };

  class EntryImageTerm : public Term {
  public:
    EntryImageTerm(const EntryImage& e,
		   unsigned& index) : _entry(e), _index(index) {}
    ~EntryImageTerm() {}
  public:
    double evaluate() const {
      double n = double(_entry.info(EntryImage::Normalization));
      double v = double(_entry.content(_index)) - 
	double(_entry.info(EntryImage::Pedestal));
      return (n>1) ? v/n : v;
    }
  private:
    const EntryImage& _entry;
    unsigned& _index;
  };
};

#endif
