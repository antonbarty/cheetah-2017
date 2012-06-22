#include "pdsdata/pnCCD/ConfigV2.hh"

#include <stdio.h>
//#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

using namespace Pds;
using namespace PNCCD;
using std::string;

ConfigV2::ConfigV2() {}

ConfigV2::ConfigV2(string sConfigFile) {
  FILE* fp = ::fopen(sConfigFile.c_str(), "r");
  size_t ret;
  if (fp) {
    printf("Reading pnCCD config file\n");
    ret = fread(&_numLinks, sizeof(ConfigV2), 1, fp);
    if (ret != 1) printf("Error reading pnCCD config file\n");
  } else {
    printf("Could not open pnCCD file\n");
  }
}

