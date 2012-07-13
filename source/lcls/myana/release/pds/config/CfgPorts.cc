#include "CfgPorts.hh"

static const int _address = 0xef2000;
using namespace Pds;

Ins CfgPorts::ins(unsigned platform)
{
  return Ins(_address+platform,10000);
}
