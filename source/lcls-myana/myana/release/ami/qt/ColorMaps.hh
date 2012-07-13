#ifndef AmiQt_ColorMaps_hh
#define AmiQt_ColorMaps_hh

#include <string>
#include <map>

using std::string;
using std::map;

namespace Ami {
  namespace Qt {
    class ColorMaps {
private:
      map<string, unsigned int *> _map;
public:
      ColorMaps();
      unsigned int *get(string colormapName);
    };
  };
};
#endif
