#include "Exception.hh"

#include <string.h>

using namespace Ami;


Event::Event(const char* who, const char* what)
{
  strncpy(_who , who , MaxLength);
  strncpy(_what, what, MaxLength);
}

const char* Event::who () const { return _who ; }
const char* Event::what() const { return _what; }




