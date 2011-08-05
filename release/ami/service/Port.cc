#include "Port.hh"

using namespace Ami;

unsigned short Port::clientPortBase() { return 5721; }
unsigned short Port::serverPort()     { return 5720; }

unsigned short Port::clientPort()     { return 5720; }
unsigned short Port::serverPortBase() { return 5721; }

unsigned Port::nPorts() { return 10; }
