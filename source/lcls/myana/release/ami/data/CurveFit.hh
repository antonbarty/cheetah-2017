#ifndef Ami_CurveFit_hh
#define Ami_CurveFit_hh

#include "ami/data/AbsOperator.hh"
#include "ami/data/DescTH1F.hh"
#include "pdsdata/xtc/ClockTime.hh"
#include <vector>

namespace Ami {

  class DescEntry;
  class EntryWaveform;
  class EntryTH1F;
  class Term;
  class FeatureCache;

  //
  //  Leading edge finder
  //
  class CurveFit : public AbsOperator {
  public:
    CurveFit(const char *name, int op, const DescEntry& output, const char *norm = 0);
    CurveFit(const char*&, const DescEntry& input, FeatureCache& features);
    ~CurveFit();
    char *name() { return _name; };
    char *norm() { return _norm; };
    int   op()   { return _op; };
  public:
    DescEntry& output   () const;
  private:
    Entry&     _operate  (const Entry&) const;
    void*      _serialize(void*) const;
  private:
    const DescEntry *_input;
    enum { NAME_LEN = 256 };
    char       _name[NAME_LEN];
    enum { DESC_LEN = 1024 };
    char       _desc_buffer[DESC_LEN];
    enum { scale=0, shift=1, chi2=2 };
    int        _op;
    enum { NORM_LEN = 256 };
    char       _norm[NORM_LEN];
    Entry*     _entry;
    std::vector<double> _refdata;
    double         _refstart, _refend;
    double         _refbw;
    Term*         _fterm;
    Term*         _nterm;

  public:
    enum { CALC_LEN = 10 };
    static struct recent {
        const DescEntry *input;
        Pds::ClockTime   time;
        double vals[3];
    } calc[CALC_LEN];
    static int calcnxt;
  };
};

#endif
