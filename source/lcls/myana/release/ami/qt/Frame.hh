#ifndef AmiQt_Frame_hh
#define AmiQt_Frame_hh

#include <QtGui/QWidget>

namespace Ami {
  namespace Qt {
    class Frame : public QWidget {
    public:
      Frame(const char* title);
      ~Frame();
    public:
    private:
    };
  };
};

#endif
