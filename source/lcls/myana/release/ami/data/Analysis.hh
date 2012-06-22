#ifndef Ami_Analysis_hh
#define Ami_Analysis_hh

namespace Ami {

  class AbsFilter;
  class AbsOperator;
  class Cds;
  class DescEntry;
  class Entry;
  class FeatureCache;

  class Analysis {
  public:
    Analysis(unsigned     id,       // server 
	     const Entry& input,    // input data to analysis
	     unsigned     output,   // output signature
	     Cds&         cds,      // repository
	     FeatureCache&,         // scalar features
	     const char*& p);       // serial stream
    ~Analysis();
  public:
    unsigned   id     () const;
    void       analyze();
    DescEntry& output () const;
  private:
    unsigned     _id;
    AbsFilter*   _filter;
    AbsOperator* _op;
    const Entry& _input;
    Cds& _cds;
  };

};

#endif
