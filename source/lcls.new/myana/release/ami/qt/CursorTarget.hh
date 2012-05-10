#ifndef AmiQt_CursorTarget_hh
#define AmiQt_CursorTarget_hh

#include "qwt_plot.h"

namespace Ami {
  namespace Qt {
    class Cursors;
    class CursorTarget {
    public:
      virtual ~CursorTarget() {}
    public:
      virtual void set_cursor_input(Cursors* c) = 0;
    };
  };
};

#endif
