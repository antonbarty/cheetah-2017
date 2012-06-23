#ifndef Ami_Ca
#define Ami_Ca

namespace Ami {

  class Ca {
  public:
    Ca(const char*);
    ~Ca();
  public:
    const char* name () const;
    double      value() const;
  };

};

#endif
