#ifndef Ami_Exception
#define Ami_Exception

namespace Ami {

  class Event {  // exception definition
  public:
    Event(const char* who, const char* what);
  public:
    const char* who () const;
    const char* what() const;
  private:
    static const unsigned MaxLength = 128;
    char _who [MaxLength];
    char _what[MaxLength];
  };

};

#endif
