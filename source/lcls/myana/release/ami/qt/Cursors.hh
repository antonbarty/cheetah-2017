#ifndef AmiQt_Cursors_hh
#define AmiQt_Cursors_hh

namespace Ami {
  namespace Qt {
    class CursorDefinition;

    class Cursors {
    public:
      virtual ~Cursors() {}
      virtual void mousePressEvent  (double, double) = 0;
      virtual void mouseMoveEvent   (double, double) = 0;
      virtual void mouseReleaseEvent(double, double) = 0;
      virtual void remove(CursorDefinition &) {};
    };
  };
};

#endif
