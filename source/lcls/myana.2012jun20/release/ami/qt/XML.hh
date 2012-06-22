#ifndef QtXML_hh
#define QtXML_hh

#include <map>

namespace Ami {
  namespace XML {
    class StartTag {
    public:
      StartTag() {}
      StartTag(const std::string element_, const std::string name_) :
        element(element_), name   (name_) {}

    public:
      std::string element;
      std::string name;
    };

    class StopTag {
    public:
      StopTag(const std::string element_) : element(element_) {}
    public:
      const std::string element;
    };

    class TagIterator {
    public:
      TagIterator(const char*&);
    public:
      bool            end() const;
      operator  const StartTag*() const;
      TagIterator&    operator++(int);
    private:
      const char*& _p;
      StartTag     _tag;
    };
  };
}

#define XML_iterate_open(pvar,tvar)                      \
  for(Ami::XML::TagIterator it(pvar); !it.end(); it++) { \
    const Ami::XML::StartTag& tvar = *it;                

#define XML_iterate_close(where,tvar)           \
  else                                          \
    printf(#where " unknown tag %s/%s\n",       \
           tvar.element.c_str(),                \
           tvar.name.c_str());                  \
  }

#endif
